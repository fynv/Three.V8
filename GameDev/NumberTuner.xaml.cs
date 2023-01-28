using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for NumberTuner.xaml
    /// </summary>
    public partial class NumberTuner : UserControl
    {
        private float _value = 0.0f;
        public float value
        {
            get
            {
                return _value;
            }
            set
            {
                if (_value != value) 
                { 
                    _value = value;
                    text.Text = _value.ToString();
                }
            }
        }

        public bool exponential = false;
        public float step = 1.0f;

        public NumberTuner()
        {
            InitializeComponent();
        }

        public event EventHandler ValueChanged;

        private void update_value()
        {
            string str = text.Text;
            float v = _value;
            float.TryParse(str, out v);
            if (v != _value)
            {
                value = v;
                ValueChanged?.Invoke(this, null);
            }

        }

        private void text_LostFocus(object sender, System.Windows.RoutedEventArgs e)
        {
            update_value();
        }

        private void text_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                update_value();
            }
        }

        private void btn_decr_Click(object sender, RoutedEventArgs e)
        {
            if (exponential)
            {
                value /= step;
            }
            else
            {
                value -= step;
            }            
            ValueChanged?.Invoke(this, null);
        }

        private void btn_incr_Click(object sender, RoutedEventArgs e)
        {
            if (exponential)
            {
                value *= step;
            }
            else
            {
                value += step;
            }
            ValueChanged?.Invoke(this, null);
        }
    }
}
