﻿using System;
using System.Windows;
using System.Windows.Media;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for DlgNewFile.xaml
    /// </summary>
    public partial class DlgNewFile : Window
    {
        private string[] type_names = new string[] { "js" };
        private string[] default_filenames = new string[] { "index.js" };
        private bool initialized = false;

        public string typename;
        public string filename;

        public DlgNewFile(Window owner)
        {
            this.Owner = owner;
            InitializeComponent();
            initialized = true;
        }

        private void btn_ok_Click(object sender, RoutedEventArgs e)
        {
            if (lst_types.SelectedIndex < 0) return;
            typename = type_names[lst_types.SelectedIndex];
            filename = text_filename.Text;
            DialogResult = true;
        }

        private void ListView_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            if (!initialized || lst_types.SelectedIndex < 0) return;            
            text_filename.Text = default_filenames[lst_types.SelectedIndex];
        }
    }
}
