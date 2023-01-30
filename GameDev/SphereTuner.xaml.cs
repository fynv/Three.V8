using System;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for SphereTuner.xaml
    /// </summary>
    public partial class SphereTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public Object3DTuner obj3d_tuner = null;

        public SphereTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];

            tuner_radius.step = 0.5f;
            tuner_radius.value = 1.0f;
            tuner_width_segments.value = 32.0f;
            tuner_height_segments.value = 16.0f;

            if (att.ContainsKey("radius"))
            {
                tuner_radius.value = float.Parse(att["radius"].ToString());
            }
            if (att.ContainsKey("widthSegments"))
            {
                tuner_width_segments.value = float.Parse(att["widthSegments"].ToString());
            }
            if (att.ContainsKey("heightSegments"))
            {
                tuner_height_segments.value = float.Parse(att["heightSegments"].ToString());
            }

            if (att.ContainsKey("is_building"))
            {
                bool is_building = att["is_building"].ToObject<bool>();
                chk_is_building.IsChecked = is_building;
            }

            obj3d_tuner = new Object3DTuner(game_player, jobj);
            stack.Children.Add(obj3d_tuner);

            var material_tuner = new SimpleMaterialTuner(game_player, jobj);
            stack.Children.Add(material_tuner);
        }

        private void tuner_raidus_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["radius"] = $"{tuner_radius.value}";

            var att = (JObject)jobj["attributes"];
            att["radius"] = tuning["radius"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_width_segments_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["widthSegments"] = $"{(int)tuner_width_segments.value}";

            var att = (JObject)jobj["attributes"];
            att["widthSegments"] = tuning["widthSegments"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_height_segments_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["heightSegments"] = $"{(int)tuner_height_segments.value}";

            var att = (JObject)jobj["attributes"];
            att["heightSegments"] = tuning["heightSegments"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void chk_is_building_Checked(object sender, System.Windows.RoutedEventArgs e)
        {
            JObject tuning = new JObject();
            tuning["is_building"] = $"{chk_is_building.IsChecked == true}";

            var att = (JObject)jobj["attributes"];
            att["is_building"] = tuning["is_building"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }
    }
}
