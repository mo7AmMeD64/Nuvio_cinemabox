// CANARIO (01/10/2026): teclado e ditado do sistema para o .tpk 6+. So entra
// no build com -p:NvTextoCanario=1 (NUVIO_TPK_TEXTO=1 bash tools/tpk.sh); a
// build de release nao compila este arquivo e o C fica com o teclado do app
// (texto_sistema_disponivel() = 0 enquanto ninguem chama
// nv_tpk_texto_registrar). NADA DISTO FOI TESTADO NUMA TV.
//
// Ponte (src/entrada_texto.c):
//   C -> host: abrir(atual, voz), fechar()   [fio do C; aqui vao ao principal]
//   host -> C: nv_tpk_texto_valor(texto inteiro), nv_tpk_texto_fim(confirmou)
//
// TECLADO: um TextField do NUI, 1x1 e transparente na janela principal (a que
// fica por baixo do GLWindow), com o foco do NUI. O DALi abre o IME da Samsung
// quando um TextField ganha foco (InputMethodContext do campo); o texto volta
// pelo TextChanged inteiro. SUSPEITO, nao medido: com o GLWindow por cima e
// com o foco de janela, o IME pode nao subir. O log diz ("[texto] host ...").
//
// DITADO: Tizen.Uix.Stt (SttClient), privilegio
// http://tizen.org/privilege/recorder (privacidade: pede permissao em
// execucao). SUSPEITO que a TV nao tenha motor de STT para terceiros (a voz da
// Samsung e o Bixby): se o Prepare nao chegar a Ready, o host registra sem
// TS_VOZ e o botao Falar nao aparece.
using System;
using System.Linq;
using System.Runtime.InteropServices;
using Tizen.NUI;
using Tizen.NUI.BaseComponents;
using Tizen.Uix.Stt;

namespace NuvioTpk
{
    class Texto
    {
        [DllImport("libnuvio.so")] static extern void nv_tpk_texto_registrar(IntPtr abrir, IntPtr fechar, int flags);
        [DllImport("libnuvio.so")] static extern void nv_tpk_texto_valor(string valor);
        [DllImport("libnuvio.so")] static extern void nv_tpk_texto_fim(int confirmou);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void FnAbrir(IntPtr atual, int voz);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void FnFechar();

        const int TS_TECLADO = 1, TS_VOZ = 2;

        readonly Action<Action> principal;
        readonly Action<string> log;
        // Referencias vivas: o C guarda os ponteiros.
        FnAbrir fAbrir; FnFechar fFechar;
        TextField campo;
        InputMethodContext imf;
        SttClient stt;
        bool aberto, sttPronto, viuPainel;

        public Texto(Action<Action> principal, Action<string> log)
        {
            this.principal = principal;
            this.log = log;
            fAbrir = (p, voz) => { string s = PtrToUtf8(p); principal(() => Abrir(s, voz != 0)); };
            fFechar = () => principal(Fechar);
            try { MontarCampo(); }
            catch (Exception e) { Log("TextField indisponivel: " + e.GetType().Name + ": " + e.Message); return; }
            Registrar();
            try { PrepararStt(); }
            catch (Exception e) { Log("STT indisponivel: " + e.GetType().Name + ": " + e.Message); }
        }

        void Registrar()
        {
            nv_tpk_texto_registrar(Marshal.GetFunctionPointerForDelegate(fAbrir),
                                   Marshal.GetFunctionPointerForDelegate(fFechar),
                                   TS_TECLADO | (sttPronto ? TS_VOZ : 0));
        }

        static string PtrToUtf8(IntPtr p)
        {
            if (p == IntPtr.Zero) return "";
            int n = 0;
            while (Marshal.ReadByte(p, n) != 0) n++;
            var b = new byte[n];
            Marshal.Copy(p, b, 0, n);
            return System.Text.Encoding.UTF8.GetString(b);
        }

        void Log(string s) { try { log("[texto] host " + s); } catch { } }

        void MontarCampo()
        {
            campo = new TextField
            {
                Size2D = new Size2D(2, 2),
                Position2D = new Position2D(0, 0),
                Opacity = 0.01f,
                Focusable = true,
                EnableSelection = false,
            };
            campo.TextChanged += (s, e) => { if (aberto) nv_tpk_texto_valor(campo.Text ?? ""); };
            campo.KeyEvent += (s, e) =>
            {
                if (!aberto || e.Key.State != Key.StateType.Down) return false;
                string k = e.Key.KeyPressedName;
                if (k == "Return" || k == "KP_Enter" || k == "Select") { Terminar(true); return true; }
                if (k == "XF86Back" || k == "Escape") { Terminar(false); return true; }
                return false;
            };
            Window.Instance.Add(campo);
            imf = campo.GetInputMethodContext();
            if (imf != null)
                imf.StatusChanged += (s, e) =>
                {
                    Log("painel do IME " + (e.StatusChanged ? "aberto" : "fechado"));
                    if (e.StatusChanged) viuPainel = true;
                    else if (aberto && viuPainel) Terminar(false);
                };
            Log("TextField pronto (imf=" + (imf != null) + ")");
        }

        void Abrir(string atual, bool voz)
        {
            aberto = true; viuPainel = false;
            campo.Text = atual;
            FocusManager.Instance.SetCurrentFocusView(campo);
            try { imf?.Activate(); imf?.ShowInputPanel(); }
            catch (Exception e) { Log("ShowInputPanel: " + e.Message); }
            Log("abrir (" + System.Text.Encoding.UTF8.GetByteCount(atual) + " bytes, voz=" + voz + ", foco=" +
                (FocusManager.Instance.GetCurrentFocusView() == campo) + ")");
            if (voz && sttPronto)
            {
                try { stt.Start(stt.DefaultLanguage, RecognitionType.Search); Log("ditado: ouvindo (" + stt.DefaultLanguage + ")"); }
                catch (Exception e) { Log("ditado: Start falhou " + e.GetType().Name + ": " + e.Message); }
            }
        }

        void Fechar()
        {
            if (!aberto) return;
            aberto = false;
            Esconder();
        }

        void Terminar(bool confirmou)
        {
            if (!aberto) return;
            aberto = false;
            nv_tpk_texto_valor(campo.Text ?? "");
            nv_tpk_texto_fim(confirmou ? 1 : 0);
            Esconder();
        }

        void Esconder()
        {
            try { imf?.HideInputPanel(); imf?.Deactivate(); } catch { }
            try { FocusManager.Instance.ClearFocus(); } catch { }
            try { if (stt != null && stt.CurrentState == State.Recording) stt.Cancel(); } catch { }
        }

        void PrepararStt()
        {
            // recorder e privilegio de privacidade: sem a permissao o Prepare
            // ou o Start falham, e o host fica sem TS_VOZ.
            try
            {
                var r = Tizen.Security.PrivacyPrivilegeManager.CheckPermission("http://tizen.org/privilege/recorder");
                Log("permissao recorder: " + r);
                if (r == Tizen.Security.CheckResult.Ask)
                    Tizen.Security.PrivacyPrivilegeManager.RequestPermission("http://tizen.org/privilege/recorder");
            }
            catch (Exception e) { Log("permissao recorder: " + e.GetType().Name + ": " + e.Message); }
            stt = new SttClient();
            stt.StateChanged += (s, e) =>
            {
                Log("STT " + e.Previous + " -> " + e.Current);
                if (e.Current == State.Ready && !sttPronto) { sttPronto = true; principal(Registrar); }
            };
            stt.RecognitionResult += (s, e) =>
            {
                string t = e.Data != null ? string.Join(" ", e.Data.Where(x => !string.IsNullOrEmpty(x))) : "";
                Log("ditado: " + e.Result + " (" + System.Text.Encoding.UTF8.GetByteCount(t) + " bytes)");
                if (e.Result != ResultEvent.FinalResult || t.Length == 0) return;
                principal(() => { if (aberto) { campo.Text = t; nv_tpk_texto_valor(t); } });
            };
            stt.ErrorOccurred += (s, e) => Log("STT erro: " + e.ErrorValue);
            stt.Prepare();
        }
    }
}
