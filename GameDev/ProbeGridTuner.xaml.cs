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
    /// Interaction logic for ProbeGridTuner.xaml
    /// </summary>
    public partial class ProbeGridTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public ProbeGridTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];
            fn_probe_data.Text = "assets/probes.dat";
            if (att.ContainsKey("probe_data"))
            {
                fn_probe_data.Text = att["probe_data"].ToString();
            }
            
            tuner_divisions.set_value(10, 5, 10);
            if (att.ContainsKey("divisions"))
            {
                string divisions = att["divisions"].ToString();
                string[] values = divisions.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_divisions.set_value(x, y, z);
            }

            tuner_coverage_min.set_value(-10, 0, -10);
            if (att.ContainsKey("coverage_min"))
            {
                string coverage_min = att["coverage_min"].ToString();
                string[] values = coverage_min.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_coverage_min.set_value(x, y, z);
            }

            tuner_coverage_max.set_value(10, 10, 10);
            if (att.ContainsKey("coverage_max"))
            {
                string coverage_max = att["coverage_max"].ToString();
                string[] values = coverage_max.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_coverage_max.set_value(x, y, z);
            }

            tuner_iterations.value = 6;
        }

        private void load_data()
        {
            JObject tuning = new JObject();
            tuning["probe_data"] = fn_probe_data.Text;

            var att = (JObject)jobj["attributes"];
            att["probe_data"] = tuning["probe_data"];

            var res = JObject.Parse(game_player.SendMessageToUser("tuning", tuning.ToString()));
            
            if (res.ContainsKey("divisions"))
            {
                att["divisions"] = res["divisions"];
                string divisions = res["divisions"].ToString();
                string[] values = divisions.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_divisions.set_value(x, y, z);
            }
            
            if (res.ContainsKey("coverage_min"))
            {
                att["coverage_min"] = res["coverage_min"];
                string coverage_min = res["coverage_min"].ToString();
                string[] values = coverage_min.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_coverage_min.set_value(x, y, z);
            }
            
            if (res.ContainsKey("coverage_max"))
            {
                att["coverage_max"] = res["coverage_max"];
                string coverage_max = res["coverage_max"].ToString();
                string[] values = coverage_max.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_coverage_max.set_value(x, y, z);
            }

        }

        private void text_LostFocus(object sender, System.Windows.RoutedEventArgs e)
        {
            load_data();
        }

        private void text_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                load_data();
            }
        }

        private void btn_browse_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "Data File(*.dat)|*.dat";
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
            fn_probe_data.Text = rel_path;
            load_data();
        }

        private void tuner_divisions_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["divisions"] = $"{tuner_divisions.x},{tuner_divisions.y},{tuner_divisions.z}";

            var att = (JObject)jobj["attributes"];
            att["divisions"] = tuning["divisions"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_coverage_min_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["coverage_min"] = $"{tuner_coverage_min.x},{tuner_coverage_min.y},{tuner_coverage_min.z}";

            var att = (JObject)jobj["attributes"];
            att["coverage_min"] = tuning["coverage_min"];

            game_player.SendMessageToUser("tuning", tuning.ToString());

        }

        private void tuner_coverage_max_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["coverage_max"] = $"{tuner_coverage_max.x},{tuner_coverage_max.y},{tuner_coverage_max.z}";

            var att = (JObject)jobj["attributes"];
            att["coverage_max"] = tuning["coverage_max"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void btn_start_Click(object sender, RoutedEventArgs e)
        {
            JObject tuning = new JObject();
            tuning["iterations"] = tuner_iterations.value;
            game_player.SendMessageToUser("generate", tuning.ToString());
        }
    }
}
