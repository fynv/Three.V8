using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using CLRBinding;
using Newtonsoft.Json.Linq;
using Microsoft.Win32;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for ModelTuner.xaml
    /// </summary>
    public partial class ModelTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public Object3DTuner obj3d_tuner = null;
        private bool initialized = false;

        public ModelTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];

            fn_source.Text = "assets/models/model.glb";
            if (att.ContainsKey("src"))
            {
                fn_source.Text = att["src"].ToString();
            }

            if (att.ContainsKey("is_building"))
            {
                bool is_building = att["is_building"].ToObject<bool>();
                chk_is_building.IsChecked = is_building;
            }

            fn_lightmap.Text = "";
            if (att.ContainsKey("lightmap"))
            {
                fn_lightmap.Text = att["lightmap"].ToString();
            }

            obj3d_tuner = new Object3DTuner(game_player, jobj);
            stack.Children.Add(obj3d_tuner);

            initialized = true;
        }

        private void load_model()
        {
            JObject tuning = new JObject();
            tuning["src"] = fn_source.Text;

            var att = (JObject)jobj["attributes"];
            att["src"] = tuning["src"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void text_LostFocus(object sender, System.Windows.RoutedEventArgs e)
        {
            load_model();
        }

        private void text_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                load_model();
            }
        }

        private void btn_browse_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "GLB Model(*.glb)|*.glb";
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
            fn_source.Text = rel_path;
            load_model();
        }

        private void chk_is_building_Checked(object sender, System.Windows.RoutedEventArgs e)
        {
            if (initialized)
            {
                JObject tuning = new JObject();
                tuning["is_building"] = $"{chk_is_building.IsChecked == true}";

                var att = (JObject)jobj["attributes"];
                att["is_building"] = tuning["is_building"];

                game_player.SendMessageToUser("tuning", tuning.ToString());
            }
        }

        private void set_lightmap()
        {
            JObject tuning = new JObject();
            tuning["lightmap"] = fn_lightmap.Text;

            var att = (JObject)jobj["attributes"];
            att["lightmap"] = tuning["lightmap"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void fn_lightmap_LostFocus(object sender, RoutedEventArgs e)
        {
            set_lightmap();
        }

        private void fn_lightmap_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                set_lightmap();
            }
        }

        private void btn_browse_lightmap_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "HDR Image(*.hdr)|*.hdr|DDS Image(*.dds)|*.dds|List(*.csv)|*.csv|RGBM(*.webp,*.png)|*.webp;*.png";
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
            fn_lightmap.Text = rel_path;
            set_lightmap();
        }

        /*private void btn_generate_lightmap_probe_vis_Click(object sender, RoutedEventArgs e)
        {
            JObject tuning = new JObject();
            game_player.SendMessageToUser("generate", tuning.ToString());
        }*/
    }
}
