using System;
using System.Text;
using System.IO;
using System.Timers;
using System.Diagnostics;
using System.Windows.Controls;
using System.Threading.Tasks;
using Newtonsoft.Json;
using System.Windows;
using Microsoft.Win32;
using CLRBinding;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for XMLEditor.xaml
    /// </summary>
    public partial class XMLEditor : UserControl, Editor
    {
        private string file_path;

        private CGLControl glControl = null;
        private CGamePlayer game_player = null;

        public XMLEditor(string file_path, string resource_root)
        {
            InitializeComponent();
            this.file_path = file_path;

            glControl = new CGLControl();
            glControl.SetFramerate(60.0f);
            glControl.Paint += GLControl_Paint;
            glControl.Dock = System.Windows.Forms.DockStyle.Fill;
            glControl.MouseDown += GLControl_MouseDown;
            glControl.MouseUp += GLControl_MouseUp;
            glControl.MouseMove += GLControl_MouseMove;
            glControl.MouseWheel += GLControl_MouseWheel;
            glControl.KeyPress += GLControl_KeyChar;
            glControl.ControlKey += GLControl_ControlKey;
            wf_host.Child = glControl;

            string exe_name = Process.GetCurrentProcess().ProcessName;
            game_player = new CGamePlayer(exe_name, glControl);

            string local_path = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            game_player.LoadScript($"{local_path}\\xmleditor\\bundle_index.js", resource_root);
            
            webView.Source = new Uri($"{local_path}/editor/xml_editor.html");
            webView.NavigationCompleted += (sender, e) =>
            {
                _doc_open(file_path);
            };
        }

        public void cleanup()
        {
            glControl.MakeCurrent();
            game_player.Dispose();
            game_player = null;
        }

        private void GLControl_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
        {
            if (game_player == null) return;
            game_player.Draw(glControl.Width, glControl.Height);
        }

        private MouseEventArgs convert_args(System.Windows.Forms.MouseEventArgs e)
        {
            int button = -1;
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                button = 0;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.Middle)
            {
                button = 1;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                button = 2;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.XButton1)
            {
                button = 3;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.XButton2)
            {
                button = 4;
            }

            MouseEventArgs args;
            args.button = button;
            args.clicks = e.Clicks;
            args.delta = e.Delta;
            args.x = e.X;
            args.y = e.Y;
            return args;
        }

        private Timer press_timer = null;
        private int x_down;
        private int y_down;

        private void GLControl_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.Focus();
            glControl.MakeCurrent();
            game_player.OnMouseDown(convert_args(e));

            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                x_down = e.X;
                y_down = e.Y;

                press_timer = new Timer();
                press_timer.Elapsed += (source, timer_event) =>
                {
                    Dispatcher.Invoke(() =>
                    {
                        press_timer = null;
                        game_player.OnLongPress(x_down, y_down);
                    });

                };
                press_timer.AutoReset = false;
                press_timer.Interval = 500.0;
                press_timer.Start();
            }
        }

        private void GLControl_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseUp(convert_args(e));

            if (press_timer != null)
            {
                press_timer.Stop();
                press_timer = null;
            }
        }

        private void GLControl_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseMove(convert_args(e));

            if (press_timer != null)
            {
                int x = e.X;
                int y = e.Y;

                int dx = x - x_down;
                int dy = y - y_down;
                double dis = Math.Sqrt((double)(dx * dx) + (double)(dy * dy));
                if (dis > 3.0)
                {
                    press_timer.Stop();
                    press_timer = null;
                }

            }
        }

        private void GLControl_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseWheel(convert_args(e));
        }

        private void GLControl_ControlKey(uint code)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnControlKey(code);
        }

        private void GLControl_KeyChar(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnChar(e.KeyChar);
            e.Handled = true;
        }


        private async Task _doc_save(string filename)
        {
            string text = await GetText();
            File.WriteAllText(filename, text, Encoding.UTF8);
        }

        private void _doc_open(string filename)
        {
            string text = File.ReadAllText(filename, Encoding.UTF8);
            SetText(text);
            game_player.SendMessageToUser("setXML", text);            
        }

        public async Task<string> doc_save_as()
        {
            var dialog = new SaveFileDialog();
            dialog.Filter = "XML(*.xml)|*.xml";
            if (dialog.ShowDialog() != true)
            {
                return "";
            }
            file_path = dialog.FileName;
            await _doc_save(file_path);
            return file_path;
        }

        public async Task doc_save()
        {
            await _doc_save(file_path);
        }

        public async Task<bool> doc_close()
        {
            bool changed = await TextChanged();
            if (changed)
            {
                var result = MessageBox.Show("File has been modified. Save it?", "Save file", MessageBoxButton.YesNoCancel);
                if (result == MessageBoxResult.Yes)
                {
                    doc_save();
                    return true;
                }
                else if (result == MessageBoxResult.No)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }

        public async Task doc_fresh()
        {
            if (await doc_close())
            {
                _doc_open(file_path);
            }

        }


        private async Task<bool> TextChanged()
        {
            string response = await webView.ExecuteScriptAsync("doc_modified()");
            return (bool)JsonConvert.DeserializeObject(response);
        }

        private async Task<string> GetText()
        {
            string response = await webView.ExecuteScriptAsync("doc_get_text()");
            return JsonConvert.DeserializeObject(response).ToString();
        }

        private async void SetText(string text)
        {
            string para = JsonConvert.SerializeObject(text);
            await webView.ExecuteScriptAsync($"doc_set_text({para})");
        }


        public void undo()
        {
            webView.ExecuteScriptAsync("editor.execCommand('undo');");
        }

        public void redo()
        {
            webView.ExecuteScriptAsync("editor.execCommand('redo');");
        }

        public void comment()
        {
            webView.ExecuteScriptAsync("editor.execCommand('togglecomment');");
        }

        public void upper()
        {
            webView.ExecuteScriptAsync("editor.execCommand('touppercase');");
        }

        public void lower()
        {
            webView.ExecuteScriptAsync("editor.execCommand('tolowercase');");
        }

        public void find()
        {
            webView.ExecuteScriptAsync("editor.execCommand('find');");
        }

        public void findnext()
        {
            webView.ExecuteScriptAsync("editor.execCommand('findnext');");
        }

        public void findprev()
        {
            webView.ExecuteScriptAsync("editor.execCommand('findprevious');");
        }

        public void replace()
        {
            webView.ExecuteScriptAsync("editor.execCommand('replace');");
        }

        public void gotoline()
        {
            webView.ExecuteScriptAsync("editor.execCommand('gotoline');");
        }
    }
}
