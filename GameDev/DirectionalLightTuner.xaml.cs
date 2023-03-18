using System;
using System.Windows.Controls;
using System.Windows.Input;
using CLRBinding;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for DirectionalLightTuner.xaml
    /// </summary>
    public partial class DirectionalLightTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public Object3DTuner obj3d_tuner = null;
        bool initialized = false;

        public DirectionalLightTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];

            tuner_intensity.step = 0.5f;
            tuner_intensity.value = 1.0f;

            if (att.ContainsKey("intensity"))
            {
                tuner_intensity.value = float.Parse(att["intensity"].ToString());
            }

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

            if (att.ContainsKey("target"))
            {
                text_target.Text = att["target"].ToString();
            }

            if (att.ContainsKey("castShadow"))
            {
                bool cast_shadow = att["castShadow"].ToObject<bool>();
                chk_cast_shadow.IsChecked = cast_shadow;
            }

            tuner_width.value = 512;
            tuner_width.step = 512;
            tuner_height.value = 512;
            tuner_height.step = 512;
            if (att.ContainsKey("size"))
            {
                string size = att["size"].ToString();
                string[] values = size.Split(',');
                tuner_width.value = float.Parse(values[0]);
                tuner_height.value = float.Parse(values[1]);
            }

            tuner_left.value = -1.0f;
            tuner_right.value = 1.0f;
            tuner_bottom.value = -1.0f;
            tuner_top.value = 1.0f;            
            tuner_near.value = 0.0f;
            tuner_far.value = 10.0f;
            if (att.ContainsKey("area"))
            {
                string area = att["area"].ToString();
                string[] values = area.Split(',');
                tuner_left.value = float.Parse(values[0]);
                tuner_right.value = float.Parse(values[1]);
                tuner_bottom.value = float.Parse(values[2]);
                tuner_top.value = float.Parse(values[3]);                
                tuner_near.value = float.Parse(values[4]);
                tuner_far.value = float.Parse(values[5]);
            }

            tuner_radius.value = 0.0f;
            tuner_radius.step = 0.05f;
            if (att.ContainsKey("radius"))
            {
                tuner_radius.value = float.Parse(att["radius"].ToString());
            }

            tuner_bias.value = 0.001f;
            tuner_bias.step = 0.001f;
            if (att.ContainsKey("bias"))
            {
                tuner_bias.value = float.Parse(att["bias"].ToString());
            }
            obj3d_tuner = new Object3DTuner(game_player, jobj);
            stack.Children.Add(obj3d_tuner);

            obj3d_tuner.tuner_rot.IsEnabled = false;
            obj3d_tuner.tuner_scale.IsEnabled = false;

            initialized = true;
        }

        private void tuner_intensity_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["intensity"] = $"{tuner_intensity.value}";

            var att = (JObject)jobj["attributes"];
            att["intensity"] = tuning["intensity"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_color_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["color"] = $"{tuner_color.r}, {tuner_color.g}, {tuner_color.b}";

            var att = (JObject)jobj["attributes"];
            att["color"] = tuning["color"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void update_target()
        {
            JObject tuning = new JObject();
            tuning["target"] = text_target.Text;

            var att = (JObject)jobj["attributes"];
            att["target"] = tuning["target"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void text_target_LostFocus(object sender, System.Windows.RoutedEventArgs e)
        {
            update_target();
        }

        private void text_target_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                update_target();
            }
        }

        private void update_shadow()
        {
            JObject tuning = new JObject();
            var att = (JObject)jobj["attributes"];
            
            tuning["castShadow"] = $"{chk_cast_shadow.IsChecked == true}";
            tuning["size"] = $"{tuner_width.value}, {tuner_height.value}";

            att["castShadow"] = tuning["castShadow"];
            att["size"] = tuning["size"];

            if (chk_cast_shadow.IsChecked == true)
            {
                tuning["area"] = $"{tuner_left.value}, {tuner_right.value}, {tuner_bottom.value}, {tuner_top.value}, {tuner_near.value}, {tuner_far.value}";
                tuning["radius"] = $"{tuner_radius.value}";
                tuning["bias"] = $"{tuner_bias.value}";

                att["area"] = tuning["area"];
                att["radius"] = tuning["radius"];
            }

            game_player.SendMessageToUser("tuning", tuning.ToString());

        }       
       
        private void chk_cast_shadow_Checked(object sender, System.Windows.RoutedEventArgs e)
        {
            grp_shadow.IsEnabled = chk_cast_shadow.IsChecked == true;

            if (initialized)
            {
                update_shadow();
            }
        }

        private void tuner_size_ValueChanged(object sender, EventArgs e)
        {
            update_shadow();
        }

        private void tuner_area_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            var att = (JObject)jobj["attributes"];

            tuning["area"] = $"{tuner_left.value}, {tuner_right.value}, {tuner_bottom.value}, {tuner_top.value}, {tuner_near.value}, {tuner_far.value}";
            att["area"] = tuning["area"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_radius_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            var att = (JObject)jobj["attributes"];

            tuning["radius"] = $"{tuner_radius.value}";
            att["radius"] = tuning["radius"];

            game_player.SendMessageToUser("tuning", tuning.ToString());

        }

        private void tuner_bias_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            var att = (JObject)jobj["attributes"];

            tuning["bias"] = $"{tuner_bias.value}";
            att["bias"] = tuning["bias"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void btn_auto_detect_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            JObject tuning = new JObject();
            tuning["auto_area"] = "auto";
            var ret = JObject.Parse(game_player.SendMessageToUser("tuning", tuning.ToString()));

            var att = (JObject)jobj["attributes"];
            att["area"] = ret["area"];
            string area = ret["area"].ToString();
            string[] values = area.Split(',');
            tuner_left.value = float.Parse(values[0]);
            tuner_right.value = float.Parse(values[1]);
            tuner_bottom.value = float.Parse(values[2]);
            tuner_top.value = float.Parse(values[3]);
            tuner_near.value = float.Parse(values[4]);
            tuner_far.value = float.Parse(values[5]);            
        }
    }
}
