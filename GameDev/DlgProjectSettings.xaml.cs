using System;
using System.Windows;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for DlgProjectSettings.xaml
    /// </summary>
    public partial class DlgProjectSettings : Window
    {
        private JObject projectData;
        
        public DlgProjectSettings(Window owner, JObject projectData)
        {
            this.Owner = owner;
            InitializeComponent();
            this.projectData = projectData;
            text_project_name.Text = projectData["project_name"].ToString();
        }

        private void btn_ok_Click(object sender, RoutedEventArgs e)
        {
            if (text_project_name.Text=="") return;
            projectData["project_name"] = text_project_name.Text;
            DialogResult = true;
        }
        
    }
}
