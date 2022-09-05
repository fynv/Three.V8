using System;
using System.IO;
using System.Text;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Diagnostics;
using System.Windows.Input;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Microsoft.Win32;
using CLRBinding;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {        
        public static RoutedCommand RoutedCommandComment = new RoutedCommand();
        public static RoutedCommand RoutedCommandUpper = new RoutedCommand();
        public static RoutedCommand RoutedCommandLower = new RoutedCommand();
        public static RoutedCommand RoutedCommandFind = new RoutedCommand();
        public static RoutedCommand RoutedCommandFindNext = new RoutedCommand();
        public static RoutedCommand RoutedCommandFindPrev = new RoutedCommand();
        public static RoutedCommand RoutedCommandReplace = new RoutedCommand();
        public static RoutedCommand RoutedCommandGoto = new RoutedCommand();
        public static RoutedCommand RoutedCommandRotate = new RoutedCommand();

        private CGLControl glControl = null;
        private CGamePlayer game_player = null;

        private string current_filename = "";
        private bool is_portrait = false;

        public MainWindow()
        {
            InitializeComponent();
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
            wfh_three.Child = glControl;

            string exe_name = Process.GetCurrentProcess().ProcessName;
            game_player = new CGamePlayer(exe_name, glControl);
            
            string local_path = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            webView.Source = new Uri($"{local_path}/editor/index.html");
            
        }        

        private bool closable = false;
        protected override async void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            if (!closable)
            {
                e.Cancel = true;
                closable = await doc_close();
                if (closable)
                {
                    Close();
                }
                return;
            }
            glControl.MakeCurrent();
            game_player.Dispose();
            game_player = null;
            base.OnClosing(e);
        }

        private void GLControl_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
        {
            if (game_player == null) return;
            game_player.Draw(glControl.Width, glControl.Height);
        }

        private CLRBinding.MouseEventArgs convert_args(System.Windows.Forms.MouseEventArgs e)
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

            CLRBinding.MouseEventArgs args;
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

        private void update_title()
        {
            if (current_filename=="")
            {
                this.Title = "GameDev - Untitled";
            }
            else
            {
                string name = Path.GetFileName(current_filename);
                this.Title = $"GameDev - {name}";
            }
        }

        private void load_script()
        {
            string dir_name = Path.GetDirectoryName(current_filename);
            string file_name = Path.GetFileName(current_filename);
            Directory.SetCurrentDirectory(dir_name);
            Process proc = new Process();
            proc.StartInfo.FileName = "cmd.exe";
            proc.StartInfo.Arguments = $"/C \"rollup.cmd {file_name} --file bundle_{file_name}\"";
            proc.StartInfo.UseShellExecute = false;
            proc.Start();
            proc.WaitForExit();

            string fn_bundle = $"{dir_name}/bundle_{file_name}";
            game_player.LoadScript(fn_bundle);
        }

        private void unload_script()
        {
            game_player.UnloadScript();
        }

        private async void _doc_save(string filename)
        {
            string text = await GetText();
            File.WriteAllText(filename, text, Encoding.UTF8);
            load_script();
        }

        private void _doc_open(string filename)
        {
            string text = File.ReadAllText(filename, Encoding.UTF8);
            SetText(text);
        }

        private bool doc_save_as()
        {
            var dialog = new SaveFileDialog();
            dialog.Filter = "Javascript|*.js";
            if (dialog.ShowDialog() != true)
            {
                return false;
            }
            current_filename = dialog.FileName;
            _doc_save(current_filename);
            update_title();            
            return true;            
        }

        private bool doc_save()
        {
            if (current_filename!="")
            {
                _doc_save(current_filename);                
                return true;
            }
            return doc_save_as();
        }

        private async Task<bool> doc_close()
        {
            bool changed = await TextChanged();           
            if (changed)
            {
                var result = MessageBox.Show("File has been modified. Save it?", "Save file", MessageBoxButton.YesNoCancel);
                if (result == MessageBoxResult.Yes)
                {
                    return doc_save();
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

        private async void doc_new()
        {
            if (await doc_close())
            {
                string text = Properties.Resources.start;
                SetText(text);
                current_filename = "";
                update_title();
                unload_script();
            }
        }

        private async void doc_open()
        {
            if (await doc_close())
            {
                var dialog = new OpenFileDialog();
                dialog.Filter = "Javascript|*.js";
                if (dialog.ShowDialog() != true) return;
                current_filename = dialog.FileName;
                _doc_open(current_filename);
                update_title();
                load_script();
            }
        }


        private void CommandNew(object sender, ExecutedRoutedEventArgs e)
        {
            doc_new();
        }

        private void CommandOpen(object sender, ExecutedRoutedEventArgs e)
        {
            doc_open();
        }

        private void CommandSave(object sender, ExecutedRoutedEventArgs e)
        {
            doc_save();
        }

        private void CommandSaveAs(object sender, ExecutedRoutedEventArgs e)
        {
            doc_save_as();
        }

        private void CommandExit(object sender, ExecutedRoutedEventArgs e)
        {
            Close();
        }

        private void CommandUndo(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('undo');");
        }

        private void CommandRedo(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('redo');");
        }      

        private void CommandComment(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('togglecomment');");
        }

        private void CommandUpper(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('touppercase');");
        }

        private void CommandLower(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('tolowercase');");
        }

        private void CommandFind(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('find');");
        }

        private void CommandFindNext(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('findnext');");
        }

        private void CommandFindPrev(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('findprevious');");
        }

        private void CommandReplace(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('replace');");
        }

        private void CommandGoto(object sender, ExecutedRoutedEventArgs e)
        {
            webView.ExecuteScriptAsync("editor.execCommand('gotoline');");
        }

        private void CommandRotate(object sender, ExecutedRoutedEventArgs e)
        {
            if (!is_portrait)
            {
                wfh_three.Width = 360;
                wfh_three.Height = 640;
                DockPanel.SetDock(wfh_three, Dock.Left);
                is_portrait = true;
                menu_rotate.Header = "To _Landscape";
            }
            else
            {

                wfh_three.Width = 640;
                wfh_three.Height = 360;
                DockPanel.SetDock(wfh_three, Dock.Top);
                is_portrait = false;
                menu_rotate.Header = "To _Portrait";
            }
        }

    }
}
