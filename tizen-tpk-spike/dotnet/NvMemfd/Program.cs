// NvMemfd — spike 2 de UEP para Tizen 4.0/5.0 (TVs 2018-2020).
//
// O NvUepProbe (spike/tizen4-nativo) rodou numa TV real (optiman, QE55Q6FNA,
// Tizen 4.0) e mostrou:
//   A. dlopen lib/   -> FALHOU  (UEP: nao carrega .so de arquivo do pacote)
//   B. dlopen data/  -> FALHOU  ("failed to map segment from shared object")
//   C. memfd         -> EXCECAO EntryPointNotFoundException: 'memfd_create'
//                       nao existe em libc.so.6  (chamamos por NOME; a libc do
//                       Tizen 4/5 NAO exporta esse simbolo)
//   D. anon-exec     -> mprotect +EXEC OK  (memoria anonima executavel PASSA)
//
// Ou seja: mmap PROT_EXEC de ARQUIVO e barrado (UEP), mas memoria ANONIMA
// executavel e liberada. A linha C falhou so por causa da chamada por nome;
// memfd_create precisa ser chamado pelo NUMERO do syscall. Em ARM 32 (armv7,
// EABI) memfd_create e o syscall 385, e o kernel dessas TVs (~Linux 4.1, tem
// memfd desde o 3.17) suporta.
//
// Este spike testa, em ordem e cada um em try/catch (uma falha nunca impede a
// proxima):
//   1. syscall(385) -> memfd anonimo, write da libnvprobe.so nele,
//      dlopen("/proc/self/fd/N"). Se resolver: nv_probe()==42 e nv_probe_thread.
//      Esta e a rota preferida (se passar, a libnuvio.so real carrega assim no
//      host NuvioTpk40).
//   2. Carregador de ELF proprio em C# (rota D): mmap anon RW, parseia os
//      program headers da libnvprobe.so, mapeia os PT_LOAD, aplica as
//      relocacoes R_ARM_RELATIVE/GLOB_DAT/JUMP_SLOT/ABS32, resolve simbolos
//      externos por dlsym(RTLD_DEFAULT), roda DT_INIT_ARRAY, mprotect +EXEC no
//      texto e chama nv_probe pelo endereco calculado do dynsym. So depende de
//      memoria anonima executavel — nao passa por nenhum inode de arquivo.
//   3. Diz na tela QUAL metodo carregou codigo nativo.
//
// Cada linha vira texto na tela E em data/tpk-host.log; o testador manda a foto.
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using ElmSharp;
using Tizen.Applications;
using Tizen.System;

namespace NvMemfd
{
    class Program : CoreUIApplication
    {
        // libc por NUMERO de syscall (a libc do Tizen 4/5 nao exporta
        // memfd_create como simbolo, so o dispatcher generico syscall()).
        [DllImport("libc.so.6", SetLastError = true, EntryPoint = "syscall")]
        static extern int syscall_memfd(int number, IntPtr name, int flags);
        [DllImport("libc.so.6", SetLastError = true, EntryPoint = "syscall")]
        static extern int syscall_cache(int number, IntPtr start, IntPtr end, int flags);
        [DllImport("libc.so.6", SetLastError = true)] static extern IntPtr write(int fd, IntPtr buf, IntPtr count);
        [DllImport("libc.so.6", SetLastError = true)] static extern int close(int fd);
        [DllImport("libc.so.6", SetLastError = true)] static extern IntPtr mmap(IntPtr addr, IntPtr len, int prot, int flags, int fd, IntPtr off);
        [DllImport("libc.so.6", SetLastError = true)] static extern int mprotect(IntPtr addr, IntPtr len, int prot);
        [DllImport("libc.so.6", SetLastError = true)] static extern int munmap(IntPtr addr, IntPtr len);
        // libdl: carga por caminho (metodo 1) e resolucao de simbolos (metodo 2).
        [DllImport("libdl.so.2")] static extern IntPtr dlopen(string path, int flags);
        [DllImport("libdl.so.2")] static extern IntPtr dlsym(IntPtr h, string sym);
        [DllImport("libdl.so.2")] static extern IntPtr dlerror();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int ProbeFn();
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void InitFn();

        const int RTLD_NOW = 2, RTLD_GLOBAL = 0x100;
        static readonly IntPtr RTLD_DEFAULT = IntPtr.Zero;   // glibc
        const int PROT_READ = 1, PROT_WRITE = 2, PROT_EXEC = 4;
        const int MAP_PRIVATE = 2, MAP_ANONYMOUS = 0x20;    // ARM/Linux
        const int SYS_memfd_create = 385;                    // ARM EABI (armv7)
        const int ARM_NR_cacheflush = 0x0f0002;              // __ARM_NR_cacheflush
        const int PAGE = 4096;
        const string Pacote = "NvMemfd";

        Window janela;
        Box coluna;
        string logPath;
        readonly List<string> linhas = new List<string>();

        protected override void OnCreate()
        {
            base.OnCreate();
            try { logPath = Path.Combine(Application.Current.DirectoryInfo.Data, "tpk-host.log"); } catch { logPath = null; }
            try { if (logPath != null) File.WriteAllText(logPath, ""); } catch { }

            janela = new Window("NvMemfd");
            janela.BackButtonPressed += (s, e) => Exit();
            var fundo = new Background(janela) { Color = Color.Black };
            fundo.Show();
            janela.AddResizeObject(fundo);
            coluna = new Box(janela) { AlignmentX = -1, AlignmentY = 0, WeightX = 1, WeightY = 1 };
            coluna.Show();
            janela.AddResizeObject(coluna);
            janela.Show();

            Linha($"Nuvio memfd probe ({Pacote}) - Tizen 4/5");
            Linha(TestaVersao());

            string raiz = Raiz();
            string src = Path.Combine(raiz, "lib", "libnvprobe.so");
            if (!File.Exists(src))
            {
                Linha("ERRO: lib/libnvprobe.so nao esta no pacote (" + src + ")");
                Rodape();
                return;
            }
            byte[] elf = null;
            try { elf = File.ReadAllBytes(src); }
            catch (Exception e) { Linha("ERRO lendo libnvprobe.so: " + e.Message); Rodape(); return; }
            Linha($"libnvprobe.so: {elf.Length} bytes");

            bool ok1 = false, ok2 = false;
            try { ok1 = Metodo1_Memfd(elf); }
            catch (Exception e) { Linha("1. memfd: EXCECAO " + e.GetType().Name + ": " + e.Message); }

            // Metodo 2 sempre roda tambem (evidencia independente): mesmo que o 1
            // passe, saber se o carregador proprio funciona vale para o plano B.
            try { ok2 = Metodo2_CarregadorElf(elf); }
            catch (Exception e) { Linha("2. loader: EXCECAO " + e.GetType().Name + ": " + e.Message); }

            if (ok1) Linha("3. VENCEU: metodo 1 (memfd syscall + /proc/self/fd). Proximo: carregar a libnuvio.so real assim no NuvioTpk40.");
            else if (ok2) Linha("3. VENCEU: metodo 2 (carregador ELF em memoria anonima).");
            else Linha("3. Nenhum metodo carregou codigo nativo. Veja as linhas 1 e 2.");

            Rodape();
        }

        void Rodape()
        {
            Linha("Mande uma FOTO desta tela na issue #137 (iqui27/nuvio-native-legacy). Voltar sai.");
        }

        // ---- Metodo 1: memfd_create pelo syscall 385 + dlopen(/proc/self/fd/N) ----
        bool Metodo1_Memfd(byte[] elf)
        {
            IntPtr nome = Marshal.StringToHGlobalAnsi("nvprobe");
            int fd;
            try { fd = syscall_memfd(SYS_memfd_create, nome, 0); }
            finally { Marshal.FreeHGlobal(nome); }
            if (fd < 0)
            {
                Linha($"1. memfd: syscall(385) FALHOU (errno={Marshal.GetLastWin32Error()})");
                return false;
            }
            Linha($"1. memfd: syscall(385) OK, fd={fd}");

            GCHandle pin = GCHandle.Alloc(elf, GCHandleType.Pinned);
            try
            {
                IntPtr baseP = pin.AddrOfPinnedObject();
                int off = 0;
                while (off < elf.Length)
                {
                    int n = (int)write(fd, baseP + off, (IntPtr)(elf.Length - off));
                    if (n <= 0) { Linha($"1. memfd: write FALHOU em {off}/{elf.Length} (errno={Marshal.GetLastWin32Error()})"); return false; }
                    off += n;
                }
            }
            finally { pin.Free(); }
            Linha($"1. memfd: escreveu {elf.Length}B no fd");

            // O fd fica ABERTO: /proc/self/fd/N so existe enquanto ele vive.
            string path = "/proc/self/fd/" + fd;
            dlerror();
            IntPtr h = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
            if (h == IntPtr.Zero)
            {
                Linha($"1. memfd: dlopen({path}) FALHOU -> {Err()}");
                return false;
            }
            int r = ChamaProbe(h, "nv_probe");
            int t = ChamaProbe(h, "nv_probe_thread");
            Linha($"1. memfd: OK! dlopen({path}) passou; nv_probe={r} (42=ok) pthread={t} (1=ok)");
            return r == 42;
        }

        int ChamaProbe(IntPtr h, string sym)
        {
            IntPtr s = dlsym(h, sym);
            if (s == IntPtr.Zero) return -1;
            return Marshal.GetDelegateForFunctionPointer<ProbeFn>(s)();
        }

        // ---- Metodo 2: carregador de ELF32 ARM em memoria anonima (rota D) ----
        // Parseia a libnvprobe.so, mapeia os PT_LOAD em anon RW, relocaliza,
        // roda DT_INIT_ARRAY, mprotect +EXEC no texto e chama nv_probe pelo
        // endereco do dynsym. Nunca toca um inode: so memoria anonima.
        bool Metodo2_CarregadorElf(byte[] e)
        {
            if (e.Length < 52 || e[0] != 0x7F || e[1] != (byte)'E' || e[2] != (byte)'L' || e[3] != (byte)'F')
            { Linha("2. loader: nao e ELF"); return false; }
            if (e[4] != 1) { Linha("2. loader: nao e ELFCLASS32"); return false; }
            if (e[5] != 1) { Linha("2. loader: nao e little-endian"); return false; }
            ushort machine = U16(e, 18);
            if (machine != 40) { Linha($"2. loader: e_machine={machine}, esperado 40 (ARM)"); return false; }

            uint phoff = U32(e, 28);
            ushort phentsize = U16(e, 42);
            ushort phnum = U16(e, 44);

            // Faixa de vaddr coberta pelos PT_LOAD.
            long minVa = long.MaxValue, maxVa = long.MinValue;
            uint dynVa = 0, dynSz = 0;
            for (int i = 0; i < phnum; i++)
            {
                int p = (int)(phoff + (uint)i * phentsize);
                uint ptype = U32(e, p);
                uint pvaddr = U32(e, p + 8);
                uint pmemsz = U32(e, p + 20);
                if (ptype == 1) // PT_LOAD
                {
                    if (pvaddr < minVa) minVa = pvaddr;
                    if (pvaddr + pmemsz > maxVa) maxVa = pvaddr + pmemsz;
                }
                else if (ptype == 2) { dynVa = pvaddr; dynSz = U32(e, p + 20); } // PT_DYNAMIC
            }
            if (minVa == long.MaxValue) { Linha("2. loader: sem PT_LOAD"); return false; }

            long baseVa = minVa & ~(long)(PAGE - 1);
            long span = ((maxVa - baseVa) + PAGE - 1) & ~(long)(PAGE - 1);

            IntPtr map = mmap(IntPtr.Zero, (IntPtr)span, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, IntPtr.Zero);
            if (map == (IntPtr)(-1) || map == IntPtr.Zero)
            { Linha($"2. loader: mmap({span}) FALHOU (errno={Marshal.GetLastWin32Error()})"); return false; }
            long delta = map.ToInt64() - baseVa;   // base de relocacao B
            Linha($"2. loader: mmap {span}B em 0x{map.ToInt64():X} (delta=0x{delta:X})");

            // Copia os PT_LOAD (o resto do memsz e BSS = anon ja zerado).
            for (int i = 0; i < phnum; i++)
            {
                int p = (int)(phoff + (uint)i * phentsize);
                if (U32(e, p) != 1) continue;
                uint poff = U32(e, p + 4);
                uint pvaddr = U32(e, p + 8);
                uint pfilesz = U32(e, p + 16);
                Marshal.Copy(e, (int)poff, (IntPtr)(delta + pvaddr), (int)pfilesz);
            }

            // Dynamic (na memoria mapeada).
            long dynAddr = delta + dynVa;
            long symtab = 0, strtab = 0, hash = 0;
            long rel = 0, relsz = 0, relent = 8;
            long jmprel = 0, pltrelsz = 0;
            long initArray = 0, initArraySz = 0, initFn = 0;
            for (long d = dynAddr; ; d += 8)
            {
                int tag = Marshal.ReadInt32((IntPtr)d);
                uint val = (uint)Marshal.ReadInt32((IntPtr)(d + 4));
                if (tag == 0) break;                 // DT_NULL
                switch (tag)
                {
                    case 4: hash = delta + val; break;        // DT_HASH
                    case 5: strtab = delta + val; break;      // DT_STRTAB
                    case 6: symtab = delta + val; break;      // DT_SYMTAB
                    case 17: rel = delta + val; break;        // DT_REL
                    case 18: relsz = val; break;              // DT_RELSZ
                    case 19: relent = val; break;             // DT_RELENT
                    case 23: jmprel = delta + val; break;     // DT_JMPREL
                    case 2: pltrelsz = val; break;            // DT_PLTRELSZ
                    case 12: initFn = delta + val; break;     // DT_INIT
                    case 25: initArray = delta + val; break;  // DT_INIT_ARRAY
                    case 27: initArraySz = val; break;        // DT_INIT_ARRAYSZ
                }
            }
            if (symtab == 0 || strtab == 0) { Linha("2. loader: sem DT_SYMTAB/DT_STRTAB"); Libera(map, span); return false; }

            // Relocacoes: DT_REL (dados) e DT_JMPREL (PLT). Em ARM ambos sao REL.
            int nrel = RelocaBloco(rel, relsz, relent, symtab, strtab, delta);
            int njmp = RelocaBloco(jmprel, pltrelsz, 8, symtab, strtab, delta);
            Linha($"2. loader: relocacoes aplicadas (rel={nrel}, plt={njmp})");

            // mprotect por segmento com as permissoes reais do PT_LOAD.
            for (int i = 0; i < phnum; i++)
            {
                int p = (int)(phoff + (uint)i * phentsize);
                if (U32(e, p) != 1) continue;
                uint pvaddr = U32(e, p + 8);
                uint pmemsz = U32(e, p + 20);
                uint pflags = U32(e, p + 24);   // PF_X=1 PF_W=2 PF_R=4
                long segStart = (delta + pvaddr) & ~(long)(PAGE - 1);
                long segEnd = ((delta + pvaddr + pmemsz) + PAGE - 1) & ~(long)(PAGE - 1);
                int prot = ((pflags & 4) != 0 ? PROT_READ : 0) | ((pflags & 2) != 0 ? PROT_WRITE : 0) | ((pflags & 1) != 0 ? PROT_EXEC : 0);
                int rc = mprotect((IntPtr)segStart, (IntPtr)(segEnd - segStart), prot);
                if (rc != 0 && (pflags & 1) != 0)
                { Linha($"2. loader: mprotect +EXEC NEGADO (errno={Marshal.GetLastWin32Error()}) -> UEP tambem barra anon exec"); Libera(map, span); return false; }
            }
            // Limpa o icache do intervalo mapeado (ARM: __ARM_NR_cacheflush).
            try { syscall_cache(ARM_NR_cacheflush, map, (IntPtr)(map.ToInt64() + span), 0); } catch { }

            // DT_INIT e DT_INIT_ARRAY.
            try
            {
                if (initFn != 0) Marshal.GetDelegateForFunctionPointer<InitFn>((IntPtr)initFn)();
                for (long a = 0; a < initArraySz; a += 4)
                {
                    long fnp = (uint)Marshal.ReadInt32((IntPtr)(initArray + a));
                    if (fnp != 0 && fnp != -1) Marshal.GetDelegateForFunctionPointer<InitFn>((IntPtr)fnp)();
                }
            }
            catch (Exception ex) { Linha("2. loader: DT_INIT falhou: " + ex.Message); }

            // Endereco de nv_probe pelo dynsym (nchain do DT_HASH = nº de simbolos).
            long addr = ResolveLocal("nv_probe", symtab, strtab, hash, delta);
            if (addr == 0) { Linha("2. loader: nv_probe nao achado no dynsym"); Libera(map, span); return false; }
            int r = Marshal.GetDelegateForFunctionPointer<ProbeFn>((IntPtr)addr)();
            Linha($"2. loader: OK! nv_probe (0x{addr:X}) retornou {r} (42=ok)");
            // Nao libera: a memoria fica valida caso o metodo 1 tenha falhado.
            return r == 42;
        }

        // Aplica um bloco de Elf32_Rel (8 bytes cada). Retorna quantas aplicou.
        int RelocaBloco(long tabela, long tamBytes, long ent, long symtab, long strtab, long delta)
        {
            if (tabela == 0 || tamBytes == 0) return 0;
            if (ent < 8) ent = 8;
            int n = 0;
            for (long o = 0; o < tamBytes; o += ent)
            {
                long r = tabela + o;
                uint rOffset = (uint)Marshal.ReadInt32((IntPtr)r);
                uint rInfo = (uint)Marshal.ReadInt32((IntPtr)(r + 4));
                int type = (int)(rInfo & 0xff);
                int symidx = (int)(rInfo >> 8);
                IntPtr where = (IntPtr)(delta + rOffset);
                switch (type)
                {
                    case 23: // R_ARM_RELATIVE: *where += B
                        Marshal.WriteInt32(where, (int)((uint)Marshal.ReadInt32(where) + (uint)delta));
                        n++;
                        break;
                    case 2:  // R_ARM_ABS32: S + A
                    case 21: // R_ARM_GLOB_DAT: S (+A)
                    {
                        uint s = (uint)ResolveSimbolo(symidx, symtab, strtab, delta);
                        uint a = (uint)Marshal.ReadInt32(where);
                        Marshal.WriteInt32(where, (int)(s + a));
                        n++;
                        break;
                    }
                    case 22: // R_ARM_JUMP_SLOT: S
                    {
                        uint s = (uint)ResolveSimbolo(symidx, symtab, strtab, delta);
                        Marshal.WriteInt32(where, (int)s);
                        n++;
                        break;
                    }
                    default:
                        // tipos nao esperados numa .so tao simples: ignora
                        break;
                }
            }
            return n;
        }

        // Resolve o valor S de um simbolo por indice: local -> delta+st_value;
        // externo (st_shndx==0) -> dlsym(RTLD_DEFAULT, nome) na libc/libEGL ja
        // carregadas.
        long ResolveSimbolo(int symidx, long symtab, long strtab, long delta)
        {
            long sym = symtab + (long)symidx * 16;
            uint stName = (uint)Marshal.ReadInt32((IntPtr)sym);
            uint stValue = (uint)Marshal.ReadInt32((IntPtr)(sym + 4));
            ushort stShndx = (ushort)Marshal.ReadInt16((IntPtr)(sym + 14));
            if (stShndx != 0) return delta + stValue;   // definido aqui
            string nome = LeCStr(strtab + stName);
            if (string.IsNullOrEmpty(nome)) return 0;
            IntPtr p = dlsym(RTLD_DEFAULT, nome);
            return p.ToInt64();                          // 0 se nao achou (weak)
        }

        // Acha um simbolo DEFINIDO localmente pelo nome, varrendo o dynsym.
        // nchain do DT_HASH da o numero de simbolos.
        long ResolveLocal(string nome, long symtab, long strtab, long hash, long delta)
        {
            int count = 0;
            if (hash != 0) count = Marshal.ReadInt32((IntPtr)(hash + 4)); // nchain
            if (count <= 0 || count > 100000) count = 64;                 // fallback
            for (int i = 0; i < count; i++)
            {
                long sym = symtab + (long)i * 16;
                uint stName = (uint)Marshal.ReadInt32((IntPtr)sym);
                uint stValue = (uint)Marshal.ReadInt32((IntPtr)(sym + 4));
                ushort stShndx = (ushort)Marshal.ReadInt16((IntPtr)(sym + 14));
                if (stShndx == 0 || stValue == 0) continue;
                if (LeCStr(strtab + stName) == nome) return delta + stValue;
            }
            return 0;
        }

        static string LeCStr(long addr)
        {
            var sb = new System.Text.StringBuilder();
            for (int i = 0; i < 256; i++)
            {
                byte b = Marshal.ReadByte((IntPtr)(addr + i));
                if (b == 0) break;
                sb.Append((char)b);
            }
            return sb.ToString();
        }

        void Libera(IntPtr map, long span) { try { munmap(map, (IntPtr)span); } catch { } }

        static uint U32(byte[] b, int o) => (uint)(b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24));
        static ushort U16(byte[] b, int o) => (ushort)(b[o] | (b[o + 1] << 8));

        static string Err()
        {
            IntPtr e = dlerror();
            string s = e == IntPtr.Zero ? null : Marshal.PtrToStringAnsi(e);
            return string.IsNullOrEmpty(s) ? "recusado sem mensagem" : s;
        }

        static string Raiz()
        {
            try { return Path.GetFullPath(Path.Combine(Application.Current.DirectoryInfo.Resource, "..")); }
            catch (Exception e) { return "?(" + e.Message + ")"; }
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
            "<font_size=26 color=#FFFFFF>" + t.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;") + "</font>";

        static void Main(string[] args)
        {
            Elementary.Initialize();
            Elementary.ThemeOverlay();
            new Program().Run(args);
        }
    }
}
