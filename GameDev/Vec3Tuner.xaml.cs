using Newtonsoft.Json.Linq;
using System;
using System.Windows.Controls;
using static System.Net.Mime.MediaTypeNames;


namespace GameDev
{
    /// <summary>
    /// Interaction logic for Vec3Tuner.xaml
    /// </summary>
    public partial class Vec3Tuner : UserControl
    {
        public float x
        {
            get
            {
                return tuner_x.value;
            }
            set
            {
                tuner_x.value = value;
            }
        }

        public float y
        {
            get
            {
                return tuner_y.value;
            }
            set
            {
                tuner_y.value = value;
            }
        }

        public float z
        {
            get
            {
                return tuner_z.value;
            }
            set
            {
                tuner_z.value = value;
            }
        }

        public void set_value(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }


        public Vec3Tuner()
        {
            InitializeComponent();
        }

        public event EventHandler ValueChanged;

        private void tuner_ValueChanged(object sender, EventArgs e)
        {            
            ValueChanged?.Invoke(sender, e);
        }
       
    }
}
