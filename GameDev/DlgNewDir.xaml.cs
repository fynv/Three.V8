using System;
using System.Windows;
using System.Windows.Media;


namespace GameDev
{
    /// <summary>
    /// Interaction logic for DlgNewDir.xaml
    /// </summary>
    public partial class DlgNewDir : Window
    {
        public string filename;

        public DlgNewDir(Window owner)
        {
            this.Owner = owner;
            InitializeComponent();
        }

        private void btn_ok_Click(object sender, RoutedEventArgs e)
        {
            if (text_filename.Text == "") return;
            filename = text_filename.Text;
            DialogResult = true;
        }
    }
}
