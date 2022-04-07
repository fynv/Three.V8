using System;
using System.Windows;
using Microsoft.Win32;
using CLRBinding;

namespace GamePlayer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private CV8VM v8vm;
        private CGLControl glControl = null;
        private CGamePlayer game_player = null;
        private string default_script = "../../../game/bundle.js";

        private void glControl_Load(object sender, System.EventArgs e)
        {
            game_player = new CGamePlayer(v8vm, glControl);       
            game_player.LoadScript(default_script);
        }

        public MainWindow(CV8VM v8vm)
        {
            this.v8vm = v8vm;
            InitializeComponent();
            glControl = new CGLControl();
            glControl.SetFramerate(60.0f);
            glControl.Load += glControl_Load;
            glControl.Paint += GLControl_Paint;
            glControl.Dock = System.Windows.Forms.DockStyle.Fill;
            glControl.MouseDown += GLControl_MouseDown;
            glControl.MouseUp += GLControl_MouseUp;
            glControl.MouseMove += GLControl_MouseMove;
            glControl.MouseWheel += GLControl_MouseWheel;
            wf_host.Child = glControl;
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

        private void BtnLoad_Click(object sender, RoutedEventArgs e)
        {
            if (game_player == null) return;
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "Script Files|*.js";
            if (openFileDialog.ShowDialog() == true)
            {
                glControl.MakeCurrent();
                game_player.LoadScript(openFileDialog.FileName);
            }
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

        private void GLControl_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseDown(convert_args(e));
        }

        private void GLControl_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseUp(convert_args(e));
        }

        private void GLControl_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseMove(convert_args(e));
        }

        private void GLControl_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseWheel(convert_args(e));
        }
    }
}
