using System;
using System.Windows;
using System.Diagnostics;
using CLRBinding;

namespace GamePlayer
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void VMMain(object data)
        {
            CV8VM v8vm = (CV8VM)data;
            var wnd = new MainWindow(v8vm);
            wnd.ShowDialog();
        }

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            this.ShutdownMode = ShutdownMode.OnExplicitShutdown;
            string exe_name = Process.GetCurrentProcess().ProcessName;
            CV8VM v8vm = new CV8VM(exe_name);
            v8vm.RunVM(VMMain, v8vm);
            v8vm.Dispose();
            Console.WriteLine("Shutting down.");
            Shutdown();
        }
    }
}
