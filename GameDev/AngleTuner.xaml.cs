using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for AngleTuner.xaml
    /// </summary>
    public partial class AngleTuner : UserControl
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
                    slider.Value = _value;
                }
            }
        }

        public AngleTuner()
        {
            InitializeComponent();
        }

        public event EventHandler ValueChanged;

        private void update_value()
        {
            string str = text.Text;
            float v = _value;
            float.TryParse(str, out v);
            while(v>180.0f)
            {
                v -= 360.0f;
            }
            while(v<-180.0f)
            {
                v += 360.0f;
            }
            if (v != _value)
            {
                value = v;
                ValueChanged?.Invoke(this, null);
            }

        }

        private void text_LostFocus(object sender, RoutedEventArgs e)
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

        private void Slider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            value = (float)slider.Value;
            ValueChanged?.Invoke(this, null);
        }
    }
}
