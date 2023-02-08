using System;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for SkyTuner.xaml
    /// </summary>
    public partial class SkyTuner : UserControl
    {
        private CGamePlayer game_player = null;
        private JObject jobj = null;
        private bool initialized = false;

        public SkyTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();
            this.game_player = game_player;
            this.jobj = jobj;

            var att = (JObject)jobj["attributes"];

            string type = "hemisphere";
            if (att.ContainsKey("type"))
            {
                type = att["type"].ToString();
            }

            if (type=="uniform")
            {
                lst_types.SelectedIndex = 0;
            }
            else if (type=="hemisphere")
            {
                lst_types.SelectedIndex = 1;
            }
            else if (type =="cube")
            {
                lst_types.SelectedIndex = 2;
            }
            else if (type == "scene")
            {
                lst_types.SelectedIndex = 3;
            }
            load_type(type);
            initialized = true;
        }

        private void load_type(string type)
        {
            property_area.Children.Clear();
            if (type == "uniform")
            {
                var tuner = new UniformSkyTuner(game_player, jobj);
                property_area.Children.Add(tuner);
            }
            else if (type == "hemisphere")
            {
                var tuner = new HemisphereSkyTuner(game_player, jobj);
                property_area.Children.Add(tuner);
            }
            else if (type == "cube")
            {
                var tuner = new CubeSkyTuner(game_player, jobj);
                property_area.Children.Add(tuner);
            }
            else if (type == "scene")
            {
                var tuner = new BackgroundSceneTuner(game_player, jobj);
                property_area.Children.Add(tuner);
            }
        }

        private void lst_types_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!initialized) return;
            if (lst_types.SelectedIndex < 0) return;     
            
            string type = ((ComboBoxItem)lst_types.SelectedItem).Tag.ToString();

            var att = (JObject)jobj["attributes"];
            att.RemoveAll();
            att["type"] = type;

            jobj["attributes"] = JObject.Parse(game_player.SendMessageToUser("tuning", att.ToString()));

            load_type(type);

        }
    }
}
