using System;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;


namespace GameDev
{
    /// <summary>
    /// Interaction logic for PlaneTuner.xaml
    /// </summary>
    public partial class PlaneTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;

        public Object3DTuner obj3d_tuner = null;

        public PlaneTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];

            tuner_width.step = 0.5f;
            tuner_width.value = 1.0f;
            tuner_height.step = 0.5f;
            tuner_height.value = 1.0f;

            if (att.ContainsKey("size"))
            {
                string size = att["size"].ToString();
                string[] values = size.Split(',');
                tuner_width.value = float.Parse(values[0]);
                tuner_height.value = float.Parse(values[1]);
            }

            obj3d_tuner = new Object3DTuner(game_player, jobj);
            stack.Children.Add(obj3d_tuner);
        }

        private void tuner_size_ValueChanged(object sender, EventArgs e)
        {
            JObject tuning = new JObject();
            tuning["size"] = $"{tuner_width.value},{tuner_height.value}";

            var att = (JObject)jobj["attributes"];
            att["size"] = tuning["size"];

            game_player.SendMessageToUser("tuning", tuning.ToString());
        }
    }
}
