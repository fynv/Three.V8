using System;
using System.Windows.Controls;
using System.Windows.Input;
using CLRBinding;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for Object3DTuner.xaml
    /// </summary>
    public partial class Object3DTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public event EventHandler UpdateName;

        public Object3DTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            tuner_pos.tuner_x.step = 0.5f;
            tuner_pos.tuner_y.step = 0.5f;
            tuner_pos.tuner_z.step = 0.5f;

            var att = (JObject)jobj["attributes"];

            if (att.ContainsKey("name"))
            {
                text_name.Text = att["name"].ToString();
            }
            else
            {
                text_name.Text = jobj["tagName"].ToString();
            }

            if (att.ContainsKey("position"))
            {
                string position = att["position"].ToString();
                string[] values = position.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_pos.set_value(x, y, z);
            }

            if (att.ContainsKey("rotation"))
            {
                string rotation = att["rotation"].ToString();
                string[] values = rotation.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_rot.set_value(x, y, z);
            }

            if (att.ContainsKey("scale"))
            {
                string rotation = att["scale"].ToString();
                string[] values = rotation.Split(',');
                float x = float.Parse(values[0]);
                float y = float.Parse(values[1]);
                float z = float.Parse(values[2]);
                tuner_scale.set_value(x, y, z);
            }
        }

        private void update_name()
        {
            JObject tuning = new JObject();
            tuning["name"] = text_name.Text;

            var att = (JObject)jobj["attributes"];
            att["name"] = tuning["name"];

            game_player.SendMessageToUser("tuning", tuning.ToString());

            UpdateName?.Invoke(this, null);
        }

        private void text_name_LostFocus(object sender, System.Windows.RoutedEventArgs e)
        {
            update_name();
        }

        private void text_name_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                update_name();
            }
        }

        private void tuner_pos_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["position"] = $"{tuner_pos.x},{tuner_pos.y},{tuner_pos.z}";

            var att = (JObject)jobj["attributes"];
            att["position"] = tuning["position"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }
        
        private void tuner_rot_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["rotation"] = $"{tuner_rot.x},{tuner_rot.y},{tuner_rot.z}";

            var att = (JObject)jobj["attributes"];
            att["rotation"] = tuning["rotation"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_scale_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["scale"] = $"{tuner_scale.x},{tuner_scale.y},{tuner_scale.z}";

            var att = (JObject)jobj["attributes"];
            att["scale"] = tuning["scale"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }
    }
}
