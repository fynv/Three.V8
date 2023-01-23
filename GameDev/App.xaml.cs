using System;
using System.Windows;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private MainWindow mainWnd = null;

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            mainWnd = new MainWindow();
            this.MainWindow = mainWnd;
            mainWnd.Show();
        }
            
        private void Application_Deactivated(object sender, EventArgs e)
        {
            if (mainWnd != null)
            {
                mainWnd.AppDeactivated();
            }           
        }

        private void Application_Activated(object sender, EventArgs e)
        {
            if (mainWnd != null)
            {
                mainWnd.AppActivated();
            }            
        }
    }
}
