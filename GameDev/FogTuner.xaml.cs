using System;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for FogTuner.xaml
    /// </summary>
    public partial class FogTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public FogTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            tuner_density.exponential = true;

            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];

            tuner_density.exponential = true;
            tuner_density.step = 1.1f;
            tuner_density.value = 0.1f;

            if (att.ContainsKey("density"))
            {
                tuner_density.value = float.Parse(att["density"].ToString());
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
        }

        private void tuner_density_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["density"] = $"{tuner_density.value}";

            var att = (JObject)jobj["attributes"];
            att["density"] = tuning["density"];

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
    }
}
