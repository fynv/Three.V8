using System;
using System.Windows;
using CLRBinding;
using Microsoft.Win32;

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

        private void glControl_Load(object sender, System.EventArgs e)
        {
            game_player = new CGamePlayer(v8vm, glControl.Width, glControl.Height);
            game_player.LoadScript("../../../game/bundle.js");
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
            wf_host.Child = glControl;
        }

        private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
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
                game_player.LoadScript(openFileDialog.FileName);
            }
        }
    }
}
