using System;
using System.IO;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Diagnostics;
using Newtonsoft.Json.Linq;
using CLRBinding;

namespace GamePlayer
{
    /// <summary>
    /// Interaction logic for PlayerWindow.xaml
    /// </summary>
    public partial class PlayerWindow : Window
    {
        private CGLControl glControl = null;
        private CGamePlayer game_player = null;


        public PlayerWindow(string path_proj, int idx)
        {            
            InitializeComponent();

            JObject obj_proj = JObject.Parse(File.ReadAllText(path_proj));
            JObject obj_target = (JObject)obj_proj["targets"][idx];

            string dir_proj = Path.GetDirectoryName(path_proj);
            string path_output = Path.GetFullPath(dir_proj + "\\" + obj_target["output"].ToString());            

            Title = obj_target["name"].ToString();

            if (obj_target.ContainsKey("width") && obj_target.ContainsKey("height"))
            {
                this.Width = int.Parse(obj_target["width"].ToString()) + 36.0;
                this.Height = int.Parse(obj_target["height"].ToString()) + 64.0;
            }

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
            game_player.SetPrintCallbacks(console_std, console_err);
            game_player.LoadScript(path_output, null);
            
        }

        private FontFamily font_courier = new FontFamily("Courier New");
        private SolidColorBrush brush_red = new SolidColorBrush(Colors.Red);

        private void console_std(string str)
        {
            string[] lines = str.Split(new char[]{'\n'});
            foreach(string str_line in lines)
            {
                var line = new TextBlock();
                line.FontFamily = font_courier;
                line.Text = str_line;
                console.Children.Add(line);
            }
            console_scroll.ScrollToBottom();
        }

        private void console_err(string str)
        {
            string[] lines = str.Split(new char[] { '\n' });
            foreach (string str_line in lines)
            {
                var line = new TextBlock();
                line.FontFamily = font_courier;
                line.Foreground = brush_red;
                line.Text = str_line;
                console.Children.Add(line);
            }
            console_scroll.ScrollToBottom();
        }

        private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
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

        private void menu_clear_console_Click(object sender, RoutedEventArgs e)
        {
            console.Children.Clear();
        }
    }
}
