using System;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;
using Microsoft.Win32;
using System.Windows.Input;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for BackgroundSceneTuner.xaml
    /// </summary>
    public partial class BackgroundSceneTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public BackgroundSceneTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];

            scene_path.Text = "terrain.xml";
            if (att.ContainsKey("scene"))
            {
                scene_path.Text = att["scene"].ToString();
            }
         
            tuner_near.value = 10.0f;
            tuner_near.step = 10.0f;
            if (att.ContainsKey("near"))
            {                
                tuner_near.value = float.Parse(att["near"].ToString());                
            }

            tuner_far.value = 10000.0f;
            tuner_far.step = 10.0f;
            if (att.ContainsKey("far"))
            {
                tuner_far.value = float.Parse(att["far"].ToString());
            }
        }

        private void update_scene()
        {
            JObject tuning = new JObject();
            tuning["scene"] = scene_path.Text;

            var att = (JObject)jobj["attributes"];
            att["scene"] = tuning["scene"];

            game_player.SendMessageToUser("tuning", tuning.ToString());

        }

        private void text_LostFocus(object sender, System.Windows.RoutedEventArgs e)
        {
            update_scene();
        }

        private void text_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                update_scene();
            }
        }

        private void btn_browse_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Multiselect = true;
            dialog.Filter = "XML(*.xml)|*.xml";
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
            scene_path.Text = rel_path;

            update_scene();
        }

        private void tuner_near_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["near"] = $"{tuner_near.value}";

            var att = (JObject)jobj["attributes"];
            att["near"] = tuning["near"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_far_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["far"] = $"{tuner_far.value}";

            var att = (JObject)jobj["attributes"];
            att["far"] = tuning["far"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }
    }
}
