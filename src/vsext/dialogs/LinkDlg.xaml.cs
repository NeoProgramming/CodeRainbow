using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace CodeRainbow
{
    /// <summary>
    /// Interaction logic for LinkDlg.xaml
    /// </summary>
    public partial class LinkDlg : Window
    {
        public string linkInfo;
        public LinkDlg(string info)
        {
            InitializeComponent();
            linkInfo = info;

            textLink.Text = info;
            textLink.Focus();
        }

        private void buttonOk_Click(object sender, RoutedEventArgs e)
        {
            linkInfo = textLink.Text;
            
            this.DialogResult = true;
            Close();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
           // Close();
        }

        private void buttonOpen_Click(object sender, RoutedEventArgs e)
        {

        }

        private void buttonPaste_Click(object sender, RoutedEventArgs e)
        {

        }

        private void buttonSelect_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
