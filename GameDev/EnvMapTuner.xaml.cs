using System;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;
using Microsoft.Win32;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for EnvMapTuner.xaml
    /// </summary>
    public partial class EnvMapTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public EnvMapTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            tuner_probe_pos.tuner_x.step = 0.5f;
            tuner_probe_pos.tuner_y.step = 0.5f;
            tuner_probe_pos.tuner_z.step = 0.5f;

            var att = (JObject)jobj["attributes"];
            
            file_path.Text = "assets/textures";
            if (att.ContainsKey("path"))
            {
                file_path.Text = att["path"].ToString();
            }

            name_posx.Text = "env_face0.jpg";
            if (att.ContainsKey("posx"))
            {
                name_posx.Text = att["posx"].ToString();
            }

            name_negx.Text = "env_face1.jpg";
            if (att.ContainsKey("negx"))
            {
                name_negx.Text = att["negx"].ToString();
            }

            name_posy.Text = "env_face2.jpg";
            if (att.ContainsKey("posy"))
            {
                name_posy.Text = att["posy"].ToString();
            }

            name_negy.Text = "env_face3.jpg";
            if (att.ContainsKey("negy"))
            {
                name_negy.Text = att["negy"].ToString();
            }

            name_posz.Text = "env_face4.jpg";
            if (att.ContainsKey("posz"))
            {
                name_posz.Text = att["posz"].ToString();
            }

            name_negz.Text = "env_face5.jpg";
            if (att.ContainsKey("negz"))
            {
                name_negz.Text = att["negz"].ToString();
            }

            if (att.ContainsKey("probe_position"))
            {
                string probe_position = att["probe_position"].ToString();
                string[] values = probe_position.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_probe_pos.set_value(x, y, z);
            }

        }

        private void tuner_probe_pos_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["probe_position"] = $"{tuner_probe_pos.x},{tuner_probe_pos.y},{tuner_probe_pos.z}";

            var att = (JObject)jobj["attributes"];
            att["probe_position"] = tuning["probe_position"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void reload()
        {
            JObject tuning = new JObject();
            tuning["path"] = file_path.Text;
            tuning["posx"] = name_posx.Text;
            tuning["negx"] = name_negx.Text;
            tuning["posy"] = name_posy.Text;
            tuning["negy"] = name_negy.Text;
            tuning["posz"] = name_posz.Text;
            tuning["negz"] = name_negz.Text;

            var att = (JObject)jobj["attributes"];
            att["path"] = tuning["path"];
            att["posx"] = tuning["posx"];
            att["negx"] = tuning["negx"];
            att["posy"] = tuning["posy"];
            att["negy"] = tuning["negy"];
            att["posz"] = tuning["posz"];
            att["negz"] = tuning["negz"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void btn_reload_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            reload();
        }

        private void btn_generate_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            JObject tuning = new JObject();
            tuning["path"] = file_path.Text;
            tuning["posx"] = name_posx.Text;
            tuning["negx"] = name_negx.Text;
            tuning["posy"] = name_posy.Text;
            tuning["negy"] = name_negy.Text;
            tuning["posz"] = name_posz.Text;
            tuning["negz"] = name_negz.Text;

            var att = (JObject)jobj["attributes"];
            att["path"] = tuning["path"];
            att["posx"] = tuning["posx"];
            att["negx"] = tuning["negx"];
            att["posy"] = tuning["posy"];
            att["negy"] = tuning["negy"];
            att["posz"] = tuning["posz"];
            att["negz"] = tuning["negz"];
            game_player.SendMessageToUser("generate", tuning.ToString());
        }

        private void btn_browse_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Multiselect = true;
            dialog.Filter = "Images(*.jpg,*.png)|*.jpg;*.png";            
            if (dialog.ShowDialog() != true) return;

            var mainwnd = Window.GetWindow(Application.Current.MainWindow) as MainWindow;
            string cur_path = mainwnd.cur_path;

            if (dialog.FileNames.Length<6)
            {
                MessageBox.Show("Exactly 6 files needed");
                return;
            }

            string dir = Path.GetDirectoryName(dialog.FileNames[0]);
            if (!dir.StartsWith(cur_path))
            {
                MessageBox.Show("Failed to parse path");
                return;
            }

            string rel_dir = dir.Substring(cur_path.Length + 1);
            file_path.Text = rel_dir;
            name_posx.Text = dialog.FileNames[0].Substring(dir.Length + 1);
            name_negx.Text = dialog.FileNames[1].Substring(dir.Length + 1);
            name_posy.Text = dialog.FileNames[2].Substring(dir.Length + 1);
            name_negy.Text = dialog.FileNames[3].Substring(dir.Length + 1);
            name_posz.Text = dialog.FileNames[4].Substring(dir.Length + 1);
            name_negz.Text = dialog.FileNames[5].Substring(dir.Length + 1);

            reload();

        }
    }
}
