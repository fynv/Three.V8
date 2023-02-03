using System;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Windows.Controls;
using System.Threading.Tasks;
using Newtonsoft.Json;
using System.Windows;
using Microsoft.Win32;
using System.Windows.Shapes;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for HelpPage.xaml
    /// </summary>
    public partial class HelpPage : UserControl, EditorBase
    {
        public HelpPage(string path)
        {
            InitializeComponent();            
            webView.Source = new Uri(path);
        }

        public void Goto(string path)
        {
            webView.Source = new Uri(path);
        }

        public void cleanup()
        {

        }

        public async Task<bool> doc_close()
        {
            return true;
        }
    }
}
