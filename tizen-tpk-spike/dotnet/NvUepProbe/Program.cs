// NvUepProbe — spike de UEP para Tizen 4.0/5.0 (TVs 2018-2020).
//
// PERGUNTA: existe alguma forma de rodar codigo C proprio dentro de um .tpk
// .NET nessas TVs, ja que o dlopen da .so em disco e recusado pela UEP
// ("failed to map segment from shared object")?
//
// O spike anterior (NvSpikeLegacy) ja provou que FALHAM: DllImport da .so
// propria (dlopen de arquivo) e Process.Start de binario estatico. Este aqui
// testa a rota que a pesquisa de seguranca de TV Samsung (califio, MADBugs,
// TV KantS2/ARMv7, mesma geracao das TVs que falharam) diz contornar a UEP:
// carregar de MEMORIA ANONIMA em vez de arquivo.
//
//   A. dlopen(lib/libnvprobe.so)         -> baseline; deve FALHAR (UEP ativa)
//   B. dlopen(data/ copia)               -> deve FALHAR (copia nao esta na SFD)
//   C. memfd_create + dlopen(/proc/self/fd/N)  -> HIPOTESE: pode PASSAR
//   D. mmap anonimo RW + mprotect +PROT_EXEC   -> a memoria exec anonima e
//                                                 permitida? (o JIT do .NET ja
//                                                 depende disso) - so checa a
//                                                 permissao, nao executa (para
//                                                 nao arriscar SIGILL antes de
//                                                 a tela aparecer)
//   E. versao/modelo da TV
//
// Se C der OK e nv_probe()==42, a libnuvio.so real pode ser carregada assim
// (ship como recurso, nunca em lib/), e o host GL passa a carregar por memfd.
// Cada linha vira texto na tela E em data/tpk-host.log: o testador manda a foto.
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using ElmSharp;
using Tizen.Applications;
using Tizen.System;

namespace NvUepProbe
{
    class Program : CoreUIApplication
    {
        // libc: memfd_create (glibc >= 2.27, o Tizen 4/5 tem), write, mmap, mprotect.
        [DllImport("libc.so.6", SetLastError = true)] static extern int memfd_create(string name, uint flags);
        [DllImport("libc.so.6", SetLastError = true, EntryPoint = "syscall")]
        static extern int syscall3(int num, IntPtr a, IntPtr b, IntPtr c);
        [DllImport("libc.so.6", SetLastError = true)] static extern IntPtr write(int fd, IntPtr buf, IntPtr count);
        [DllImport("libc.so.6", SetLastError = true)] static extern IntPtr mmap(IntPtr addr, IntPtr len, int prot, int flags, int fd, IntPtr off);
        [DllImport("libc.so.6", SetLastError = true)] static extern int mprotect(IntPtr addr, IntPtr len, int prot);
        [DllImport("libc.so.6", SetLastError = true)] static extern int munmap(IntPtr addr, IntPtr len);
        // libdl: carga por caminho.
        [DllImport("libdl.so.2")] static extern IntPtr dlopen(string path, int flags);
        [DllImport("libdl.so.2")] static extern IntPtr dlsym(IntPtr h, string sym);
        [DllImport("libdl.so.2")] static extern IntPtr dlerror();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int ProbeFn();

        const int RTLD_NOW = 2, RTLD_GLOBAL = 0x100;
        const int PROT_READ = 1, PROT_WRITE = 2, PROT_EXEC = 4;
        const int MAP_PRIVATE = 2, MAP_ANONYMOUS = 0x20;   // ARM/Linux
        const int SYS_memfd_create_arm = 385;              // EABI
        const string Pacote = "NvUepProbe";

        Window janela;
        Box coluna;
        string logPath;
        readonly List<string> linhas = new List<string>();

        protected override void OnCreate()
        {
            base.OnCreate();
            try { logPath = Path.Combine(Application.Current.DirectoryInfo.Data, "tpk-host.log"); } catch { logPath = null; }
            try { if (logPath != null) File.WriteAllText(logPath, ""); } catch { }

            janela = new Window("NvUepProbe");
            janela.BackButtonPressed += (s, e) => Exit();
            var fundo = new Background(janela) { Color = Color.Black };
            fundo.Show();
            janela.AddResizeObject(fundo);
            coluna = new Box(janela) { AlignmentX = -1, AlignmentY = 0, WeightX = 1, WeightY = 1 };
            coluna.Show();
            janela.AddResizeObject(coluna);
            janela.Show();

            Linha($"Nuvio UEP probe ({Pacote}) - Tizen 4/5");
            Linha(TestaVersao());
            string raiz = Raiz();
            Linha("A. " + TestaDlopenArquivo(Path.Combine(raiz, "lib", "libnvprobe.so"), "lib/"));
            Linha("B. " + TestaDlopenCopia(raiz));
            Linha("C. " + TestaMemfd(raiz));
            Linha("D. " + TestaAnonExec());
            Linha("Mande uma FOTO desta tela na issue #137 (iqui27/nuvio-native-legacy).");
            Linha("A linha C e a que importa. Voltar sai.");
        }

        static string Raiz()
        {
            try { return Path.GetFullPath(Path.Combine(Application.Current.DirectoryInfo.Resource, "..")); }
            catch (Exception e) { return "?(" + e.Message + ")"; }
        }

        // A/baseline: dlopen de arquivo no pacote. Esperado: FALHA (UEP).
        string TestaDlopenArquivo(string p, string rotulo)
        {
            try
            {
                if (!File.Exists(p)) return $"dlopen {rotulo}: arquivo nao existe ({p})";
                dlerror();
                IntPtr h = dlopen(p, RTLD_NOW | RTLD_GLOBAL);
                if (h == IntPtr.Zero) return $"dlopen {rotulo}: FALHOU -> {Err()}";
                return $"dlopen {rotulo}: OK (inesperado numa TV com UEP) nv_probe={ChamaProbe(h)}";
            }
            catch (Exception e) { return $"dlopen {rotulo}: EXCECAO {e.GetType().Name}: {e.Message}"; }
        }

        // B: copia para data/ (particao rw) e dlopen de la. Esperado: FALHA
        // (a copia nao esta na base de assinaturas do pacote).
        string TestaDlopenCopia(string raiz)
        {
            try
            {
                string src = Path.Combine(raiz, "lib", "libnvprobe.so");
                if (!File.Exists(src)) return "dlopen data/: fonte lib/libnvprobe.so nao existe";
                string dst = Path.Combine(Application.Current.DirectoryInfo.Data, "libnvprobe.so");
                File.Copy(src, dst, true);
                dlerror();
                IntPtr h = dlopen(dst, RTLD_NOW | RTLD_GLOBAL);
                if (h == IntPtr.Zero) return $"dlopen data/: FALHOU -> {Err()}";
                return $"dlopen data/: OK nv_probe={ChamaProbe(h)}";
            }
            catch (Exception e) { return $"dlopen data/: EXCECAO {e.GetType().Name}: {e.Message}"; }
        }

        // C: memfd_create -> write bytes -> dlopen("/proc/self/fd/N"). A .so
        // nunca toca um inode de arquivo; a UEP nao tem o que checar. O fd fica
        // ABERTO de proposito (fechar cedo faz o dlopen do proximo pegar cache).
        string TestaMemfd(string raiz)
        {
            try
            {
                string src = Path.Combine(raiz, "lib", "libnvprobe.so");
                if (!File.Exists(src)) return "memfd: fonte lib/libnvprobe.so nao existe";
                byte[] bytes = File.ReadAllBytes(src);

                int fd = memfd_create("nvprobe", 0);
                if (fd < 0)
                {
                    // fallback: syscall direto (EABI 385)
                    IntPtr nome = Marshal.StringToHGlobalAnsi("nvprobe");
                    fd = syscall3(SYS_memfd_create_arm, nome, IntPtr.Zero, IntPtr.Zero);
                    Marshal.FreeHGlobal(nome);
                    if (fd < 0) return $"memfd: memfd_create FALHOU (errno={Marshal.GetLastWin32Error()})";
                }

                GCHandle pin = GCHandle.Alloc(bytes, GCHandleType.Pinned);
                try
                {
                    IntPtr baseP = pin.AddrOfPinnedObject();
                    int off = 0;
                    while (off < bytes.Length)
                    {
                        IntPtr w = write(fd, baseP + off, (IntPtr)(bytes.Length - off));
                        int n = (int)w;
                        if (n <= 0) return $"memfd: write FALHOU em {off}/{bytes.Length} (errno={Marshal.GetLastWin32Error()})";
                        off += n;
                    }
                }
                finally { pin.Free(); }

                string path = "/proc/self/fd/" + fd;
                dlerror();
                IntPtr h = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
                if (h == IntPtr.Zero) return $"memfd: dlopen({path}) FALHOU -> {Err()} [escreveu {bytes.Length}B]";
                int r = ChamaProbe(h);
                int t = ChamaProbeThread(h);
                return $"memfd: OK! dlopen do /proc/self/fd/{fd} passou; nv_probe={r} (42=ok) pthread={t} (1=ok)";
            }
            catch (Exception e) { return $"memfd: EXCECAO {e.GetType().Name}: {e.Message}"; }
        }

        // D: a memoria anonima pode virar executavel? So checa a PERMISSAO
        // (mprotect +PROT_EXEC == 0). NAO executa nada escrito a mao: um SIGILL
        // por cache de instrucao apagaria a tela antes da foto. Se isto passa,
        // um carregador de ELF proprio em memoria anonima e viavel.
        string TestaAnonExec()
        {
            IntPtr m = IntPtr.Zero;
            try
            {
                IntPtr len = (IntPtr)4096;
                m = mmap(IntPtr.Zero, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, IntPtr.Zero);
                if (m == (IntPtr)(-1) || m == IntPtr.Zero) return $"anon-exec: mmap FALHOU (errno={Marshal.GetLastWin32Error()})";
                int rc = mprotect(m, len, PROT_READ | PROT_EXEC);
                if (rc != 0) return $"anon-exec: mprotect +EXEC NEGADO (errno={Marshal.GetLastWin32Error()}) -> carregador proprio inviavel";
                return "anon-exec: mprotect +EXEC OK (memoria anonima executavel permitida)";
            }
            catch (Exception e) { return $"anon-exec: EXCECAO {e.GetType().Name}: {e.Message}"; }
            finally { if (m != IntPtr.Zero && m != (IntPtr)(-1)) try { munmap(m, (IntPtr)4096); } catch { } }
        }

        int ChamaProbe(IntPtr h)
        {
            IntPtr s = dlsym(h, "nv_probe");
            if (s == IntPtr.Zero) return -1;
            return Marshal.GetDelegateForFunctionPointer<ProbeFn>(s)();
        }

        int ChamaProbeThread(IntPtr h)
        {
            IntPtr s = dlsym(h, "nv_probe_thread");
            if (s == IntPtr.Zero) return -1;
            return Marshal.GetDelegateForFunctionPointer<ProbeFn>(s)();
        }

        static string Err()
        {
            IntPtr e = dlerror();
            string s = e == IntPtr.Zero ? null : Marshal.PtrToStringAnsi(e);
            return string.IsNullOrEmpty(s) ? "recusado sem mensagem" : s;
        }

        string TestaVersao()
        {
            try
            {
                Information.TryGetValue("http://tizen.org/feature/platform.version", out string v);
                Information.TryGetValue("http://tizen.org/system/model_name", out string m);
                return $"TV {m ?? "?"} / Tizen {v ?? "?"} / {RuntimeInformation.FrameworkDescription}";
            }
            catch (Exception e) { return $"versao: {e.GetType().Name}: {e.Message}"; }
        }

        void Linha(string texto)
        {
            var l = new Label(janela) { Text = Marcado(texto), AlignmentX = -1, WeightX = 1, LineWrapType = WrapType.Word };
            l.Show();
            coluna.PackEnd(l);
            linhas.Add(texto);
            try { if (logPath != null) File.AppendAllText(logPath, texto + "\n"); } catch { }
        }

        static string Marcado(string t) =>
            "<font_size=28 color=#FFFFFF>" + t.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;") + "</font>";

        static void Main(string[] args)
        {
            Elementary.Initialize();
            Elementary.ThemeOverlay();
            new Program().Run(args);
        }
    }
}
