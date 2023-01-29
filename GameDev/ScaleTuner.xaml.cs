using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using static System.Net.Mime.MediaTypeNames;
using Newtonsoft.Json.Linq;


namespace GameDev
{
    /// <summary>
    /// Interaction logic for ScaleTuner.xaml
    /// </summary>
    public partial class ScaleTuner : UserControl
    {
        private float _x = 1.0f;
        public float x
        {
            get
            {
                return _x;
            }
            set
            {
                if (_x != value)
                {
                    _x = value;
                    text_x.Text = _x.ToString();
                }
            }
        }

        private float _y = 1.0f;
        public float y
        {
            get
            {
                return _y;
            }
            set
            {
                if (_y != value)
                {
                    _y = value;
                    text_y.Text = _y.ToString();
                }
            }
        }

        private float _z = 1.0f;
        public float z
        {
            get
            {
                return _z;
            }
            set
            {
                if (_z != value)
                {
                    _z = value;
                    text_z.Text = _z.ToString();
                }
            }
        }

        public float step = 1.1f;

        public void set_value(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public ScaleTuner()
        {
            InitializeComponent();
        }

        public event EventHandler ValueChanged;

        private void update_value_x()
        {
            string str = text_x.Text;
            float v = _x;
            float.TryParse(str, out v);
            if (v != _x)
            {
                x = v;
                ValueChanged?.Invoke(this, null);
            }
        }

        private void text_x_LostFocus(object sender, RoutedEventArgs e)
        {
            update_value_x();
        }

        private void text_x_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                update_value_x();
            }
        }

        private void update_value_y()
        {
            string str = text_y.Text;
            float v = _y;
            float.TryParse(str, out v);
            if (v != _y)
            {
                y = v;
                ValueChanged?.Invoke(this, null);
            }
        }

        private void text_y_LostFocus(object sender, RoutedEventArgs e)
        {
            update_value_y();
        }

        private void text_y_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                update_value_y();
            }
        }

        private void update_value_z()
        {
            string str = text_z.Text;
            float v = _z;
            float.TryParse(str, out v);
            if (v != _z)
            {
                z = v;
                ValueChanged?.Invoke(this, null);
            }
        }

        private void text_z_LostFocus(object sender, RoutedEventArgs e)
        {
            update_value_z();
        }

        private void text_z_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                update_value_z();
            }
        }

        private void btn_decr_Click(object sender, RoutedEventArgs e)
        {
            x /= step;
            y /= step;
            z /= step;
            ValueChanged?.Invoke(this, null);
        }

        private void btn_incr_Click(object sender, RoutedEventArgs e)
        {
            x *= step;
            y *= step;
            z *= step;
            ValueChanged?.Invoke(this, null);
        }

    }
}
