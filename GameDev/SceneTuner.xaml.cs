using System.Windows;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for SceneTuner.xaml
    /// </summary>
    public partial class SceneTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;


        public SceneTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            tuner_iterations.value = 6;
        }

        private void btn_bake_Click(object sender, RoutedEventArgs e)
        {
            JObject tuning = new JObject();
            tuning["iterations"] = tuner_iterations.value;
            game_player.SendMessageToUser("generate", tuning.ToString());
        }
    }
}
