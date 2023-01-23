using System;
using System.Windows;
using Microsoft.Win32;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for DlgEditTarget.xaml
    /// </summary>
    public partial class DlgEditTarget : Window
    {
        private string root;
        public JObject jTarget;

        public DlgEditTarget(Window owner, JObject jTarget, string root)
        {
            this.root = root;
            this.Owner = owner;
            InitializeComponent();

            if (jTarget != null ) 
            {
                Title = "Edit Target";
                this.jTarget = jTarget;
            }
            else
            {
                Title = "Add Target";
                this.jTarget = new JObject();
                this.jTarget["name"] = "Target";
                this.jTarget["input"] = "index.js";
                this.jTarget["output"] = "bundle_index.js";
                this.jTarget["dirty"] = true;
            }

            text_name.Text = this.jTarget["name"].ToString();
            text_input.Text = this.jTarget["input"].ToString();
            text_output.Text = this.jTarget["output"].ToString();
        }

        private void btn_ok_Click(object sender, RoutedEventArgs e)
        {
            if (text_name.Text == "" || text_input.Text == "") return;
            jTarget["name"] = text_name.Text;
            jTarget["input"] = text_input.Text;
            jTarget["output"] = text_output.Text;
            DialogResult = true;
        }

        private void text_input_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        {
            if (!text_input.Text.EndsWith(".js"))
            {
                int idx = text_input.CaretIndex;
                if (text_input.Text.EndsWith(".j"))
                {
                    text_input.Text += "s";
                }
                else if (text_input.Text.EndsWith("."))
                {
                    text_input.Text += "js";
                }
                else
                {
                    text_input.Text += ".js";
                }
                text_input.CaretIndex = idx;
            }
            text_output.Text = "bundle_" + text_input.Text;
        }

        private void btn_browse_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "JavaScript(*.js)|*.js";
            if (dialog.ShowDialog() != true) return;
            if (dialog.FileName.StartsWith(root))
            {
                text_input.Text = dialog.FileName.Substring(root.Length + 1);
            }
            else
            {
                MessageBox.Show("File out of project scope: " + dialog.FileName, "Wrong file", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
        }
    }
}
