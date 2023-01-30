using System;
using System.Windows;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;
using Microsoft.Win32;
using System.Windows.Input;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for SimpleMaterialTuner.xaml
    /// </summary>
    public partial class SimpleMaterialTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public SimpleMaterialTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];
            float r = 1.0f;
            float g = 1.0f;
            float b = 1.0f;
            if (att.ContainsKey("color"))
            {
                var color = att["color"].ToString().Split(',');
                r = float.Parse(color[0]);
                g = float.Parse(color[1]);
                b = float.Parse(color[2]);
            }
            tuner_color.set_color(r, g, b);
            
            if (att.ContainsKey("texture"))
            {
                fn_texture.Text = att["texture"].ToString();
            }

            tuner_metalness.step = 0.1f;
            tuner_metalness.value = 0.0f;

            if (att.ContainsKey("metalness"))
            {
                tuner_metalness.value = float.Parse(att["metalness"].ToString());
            }

            tuner_roughness.step = 0.1f;
            tuner_roughness.value = 1.0f;

            if (att.ContainsKey("roughness"))
            {
                tuner_roughness.value = float.Parse(att["roughness"].ToString());
            }
        }

        private void tuner_color_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["color"] = $"{tuner_color.r}, {tuner_color.g}, {tuner_color.b}";

            var att = (JObject)jobj["attributes"];
            att["color"] = tuning["color"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void load_texture()
        {
            JObject tuning = new JObject();
            tuning["texture"] = fn_texture.Text;        

            var att = (JObject)jobj["attributes"];
            att["texture"] = tuning["texture"];           

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void text_LostFocus(object sender, System.Windows.RoutedEventArgs e)
        {
            load_texture();
        }

        private void text_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                load_texture();
            }
        }

        private void btn_browse_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();            
            dialog.Filter = "Images(*.jpg,*.png)|*.jpg;*.png";
            if (dialog.ShowDialog() != true) return;

            var mainwnd = Window.GetWindow(Application.Current.MainWindow) as MainWindow;
            string cur_path = mainwnd.cur_path;

            string path = dialog.FileName;
            if (!path.StartsWith(cur_path))
            {
                MessageBox.Show("Failed to parse path");
                return;
            }

            string rel_path = path.Substring(cur_path.Length + 1);
            fn_texture.Text = rel_path;
            load_texture();
        }

        private void tuner_metalness_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["metalness"] = $"{tuner_metalness.value}";

            var att = (JObject)jobj["attributes"];
            att["metalness"] = tuning["metalness"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_roughness_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["roughness"] = $"{tuner_roughness.value}";

            var att = (JObject)jobj["attributes"];
            att["roughness"] = tuning["roughness"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }
    }
}
