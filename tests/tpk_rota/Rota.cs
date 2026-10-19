// tests/tpk_rota.sh (#184): "libnuvio.so" ao lado do executavel faz o papel da
// empacotada (lib/ do pacote); a "encenada" e outra .so, aberta por caminho
// como a memfd. Sem rota o DllImport cai na empacotada (o bug); com
// NvCarga.RotearDllImport cai na encenada.
using System;
using System.Runtime.InteropServices;
using NuvioTpk;

static class Rota
{
    [DllImport("libnuvio.so")] static extern int nv_teste_versao();

    static int Main(string[] a)
    {
        IntPtr h = NativeLibrary.Load(a[1]);   // a "encenada", ja no processo
        if (a[0] == "com")
        {
            if (!NvCarga.PodeRotear()) { Console.WriteLine("sem rota no runtime"); return 2; }
            if (!NvCarga.RotearDllImport(typeof(Rota).Assembly, h, out string f)) { Console.WriteLine("rota falhou: " + f); return 2; }
        }
        Console.WriteLine("versao=" + nv_teste_versao() + " rotas=" + NvCarga.RotaUsos);
        return 0;
    }
}
