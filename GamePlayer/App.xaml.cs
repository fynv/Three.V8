using System;
using System.IO;
using System.Windows;

namespace GamePlayer
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            if (e.Args.Length > 1)
            {                
                string path_proj = e.Args[0];
                int idx = int.Parse(e.Args[1]);                
                this.MainWindow = new PlayerWindow(path_proj, idx);
            }
            else if (File.Exists(".\\client\\project.json"))
            {
                this.MainWindow = new PlayerWindow(".\\client\\project.json", 0);
            }
            else
            {                
                this.MainWindow = new MainWindow();
            }

            this.MainWindow.Show();
        }
    }
}
