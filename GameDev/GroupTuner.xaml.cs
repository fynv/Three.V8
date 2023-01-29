using System;
using System.Windows.Controls;
using CLRBinding;
using Newtonsoft.Json.Linq;


namespace GameDev
{
    /// <summary>
    /// Interaction logic for GroupTuner.xaml
    /// </summary>
    public partial class GroupTuner : UserControl
    {
        public Object3DTuner obj3d_tuner = null;

        public GroupTuner(CGamePlayer game_player, JObject jobj)
        {
            InitializeComponent();    
            obj3d_tuner = new Object3DTuner(game_player, jobj);
            stack.Children.Add(obj3d_tuner);
        }
    }
}
