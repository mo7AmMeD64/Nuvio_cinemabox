// "Onde assistir" do Nuvio .tpk (src/ondever.c): lista os installed apps,
// abre um deles, ou abre a loja da Samsung na pagina de um app. O C pede pelas
// funcoes registradas em nv_tpk_apps_registrar; a lista volta app por app em
// nv_tpk_app, de um fio do pool (o C guarda sob trava).
using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using Tizen.Applications;

namespace NuvioTpk
{
    static class Apps
    {
        [DllImport("libnuvio.so")] static extern void nv_tpk_apps_registrar(IntPtr listar, IntPtr abrir, IntPtr loja);
        [DllImport("libnuvio.so")] static extern void nv_tpk_app(string id, string nome);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void RegisterNative(IntPtr list, IntPtr launch, IntPtr store);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void AppNative(string id, string name);
        static RegisterNative registerNative=nv_tpk_apps_registrar;
        static AppNative appNative=nv_tpk_app;

        // TPK40's ELF loader doesn't put the library in the soname link map.
        // Resolve these exports through the same route as the video bridge.
        public static void Ligar(Func<string,IntPtr> resolve) {
            IntPtr register=resolve("nv_tpk_apps_registrar"), app=resolve("nv_tpk_app");
            if(register!=IntPtr.Zero) registerNative=Marshal.GetDelegateForFunctionPointer<RegisterNative>(register);
            if(app!=IntPtr.Zero) appNative=Marshal.GetDelegateForFunctionPointer<AppNative>(app);
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void FnSemArg();
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void FnTexto(IntPtr texto);

        // Referencias vivas: o C guarda os ponteiros, o GC nao pode recolher.
        static FnSemArg fListar;
        static FnTexto fAbrir, fLoja;

        public static void Registrar(Action<Action> post)
        {
            fListar = () => Task.Run(Listar);
            fAbrir = p => { string id = Marshal.PtrToStringAnsi(p); post(() => Lancar(id)); };
            fLoja = p => { string id = Marshal.PtrToStringAnsi(p); post(() => Loja(id)); };
            registerNative(Marshal.GetFunctionPointerForDelegate(fListar),
                                  Marshal.GetFunctionPointerForDelegate(fAbrir),
                                  Marshal.GetFunctionPointerForDelegate(fLoja));
        }

        static async Task Listar()
        {
            try
            {
                int n = 0;
                foreach (var a in await ApplicationManager.GetInstalledApplicationsAsync())
                    if (!string.IsNullOrEmpty(a.Label) && !string.IsNullOrEmpty(a.ApplicationId)) { appNative(a.ApplicationId, a.Label); n++; }
                Log($"{n} installed apps");
            }
            catch (Exception e) { Log("list apps: " + e.Message); }
        }

        static void Lancar(string id)
        {
            try { AppControl.SendLaunchRequest(new AppControl { ApplicationId = id, Operation = AppControlOperations.Default }); }
            catch (Exception e) { Log("launch " + id + ": " + e.Message); }
        }

        // A loja (org.volt.apps) abre na pagina do app com Sub_Menu=detail e
        // widget_id; sem id, na vitrine.
        static void Loja(string id)
        {
            try
            {
                var c = new AppControl { ApplicationId = "org.volt.apps", Operation = AppControlOperations.Default };
                if (!string.IsNullOrEmpty(id)) { c.ExtraData.Add("Sub_Menu", "detail"); c.ExtraData.Add("widget_id", id); }
                AppControl.SendLaunchRequest(c);
            }
            catch (Exception e) { Log("store " + id + ": " + e.Message); }
        }

        static void Log(string s) { try { NvVid.LogNativo("[apps] " + s); } catch { } }
    }
}
