using System;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for HemisphereLightTuner.xaml
    /// </summary>
    public partial class HemisphereLightTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public HemisphereLightTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];

            {
                float r = 0.318f;
                float g = 0.318f;
                float b = 0.318f;
                if (att.ContainsKey("skyColor"))
                {
                    var color = att["skyColor"].ToString().Split(',');
                    r = float.Parse(color[0]);
                    g = float.Parse(color[1]);
                    b = float.Parse(color[2]);
                }
                tuner_sky_color.set_color(r, g, b);
            }
            {
                float r = 0.01f;
                float g = 0.025f;
                float b = 0.025f;
                if (att.ContainsKey("groundColor"))
                {
                    var color = att["groundColor"].ToString().Split(',');
                    r = float.Parse(color[0]);
                    g = float.Parse(color[1]);
                    b = float.Parse(color[2]);
                }
                tuner_ground_color.set_color(r, g, b);
            }
        }

        private void tuner_sky_color_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["skyColor"] = $"{tuner_sky_color.r}, {tuner_sky_color.g}, {tuner_sky_color.b}";

            var att = (JObject)jobj["attributes"];
            att["skyColor"] = tuning["skyColor"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }

        private void tuner_ground_color_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["groundColor"] = $"{tuner_ground_color.r}, {tuner_ground_color.g}, {tuner_ground_color.b}";

            var att = (JObject)jobj["attributes"];
            att["groundColor"] = tuning["groundColor"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }
    }
}
