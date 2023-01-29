using System;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for AmbientLightTuner.xaml
    /// </summary>
    public partial class AmbientLightTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public AmbientLightTuner(CGamePlayer game_player, JObject jobj)
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
