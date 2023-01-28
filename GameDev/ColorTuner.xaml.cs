using System;
using System.Windows.Controls;
using System.Windows.Media;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for ColorTuner.xaml
    /// </summary>
    public partial class ColorTuner : UserControl
    {
        private float _r = 0.0f;
        public float r
        {
            get
            {
                return _r;
            }
            set
            {
                if (_r != value)
                {
                    _r = value;
                    update();
                }
            }
        }

        private float _g = 0.0f;
        public float g
        {
            get
            {
                return _g;
            }
            set
            {
                if (_g != value)
                {
                    _g = value;
                    update();
                }
            }
        }

        private float _b = 0.0f;
        public float b
        {
            get
            {
                return _b;
            }
            set
            {
                if (_b != value)
                {
                    _b = value;
                    update();
                }
            }
        }

        public void set_color(float r, float g, float b)
        {
            _r = r;
            _g = g;
            _b = b;
            update();
        }

        private void update()
        {
            Color col = Color.FromScRgb(1.0f, r, g, b);
            btn.Foreground = new SolidColorBrush(col);
        }

        public ColorTuner()
        {
            InitializeComponent();
        }

        public event EventHandler ValueChanged;

        private void btn_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            Color col_in = Color.FromScRgb(1.0f, r, g, b);

            var dlg = new System.Windows.Forms.ColorDialog();
            dlg.Color = System.Drawing.Color.FromArgb(col_in.A, col_in.R, col_in.G, col_in.B);
            if (dlg.ShowDialog()== System.Windows.Forms.DialogResult.OK)
            {
                var col_dlg = dlg.Color;
                Color col_out = Color.FromArgb(col_dlg.A, col_dlg.R, col_dlg.G, col_dlg.B);
                set_color(col_out.ScR, col_out.ScG, col_out.ScB);
                ValueChanged?.Invoke(this, null);
            }
        }
    }
}
