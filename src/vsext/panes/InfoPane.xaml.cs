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
using System.Windows.Navigation;
using System.Windows.Shapes;
using CodeRainbow;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;



namespace CodeRainbow
{
    /// <summary>
    /// Interaction logic for InfoPane.xaml
    /// </summary>
    public partial class InfoPane : System.Windows.Controls.UserControl
    {
        // data
        UIntPtr nodeText = UIntPtr.Zero;
        string text = "";

        public InfoPane()
        {
            InitializeComponent();
            CRPlugin.Inst.paneInfo = this;
            loadText(UIntPtr.Zero);
        }

        public void loadText(UIntPtr node)
        {
            nodeText = node;
            if (nodeText == UIntPtr.Zero)
            {
                text = "";
                textInfo.Text = "";
                textInfo.IsEnabled = false;
                textTitle.Text = "";
                textTitle.Background = new SolidColorBrush(Colors.Yellow);
            }
            else
            {
                text = CRInterop.getNodeText(nodeText);
                textInfo.Text = text;
                textInfo.IsEnabled = true;
                textTitle.Text = CRInterop.getNodeFullName(nodeText);
                textTitle.Background = new SolidColorBrush(Colors.LimeGreen);
            }
        }

        private void textInfo_LostFocus(object sender, RoutedEventArgs e)
        {
            if(text != textInfo.Text)
                CRInterop.setNodeText(nodeText, textInfo.Text);
        }

        private void onButtonTreeClick(object sender, RoutedEventArgs e)
        {
            CRPlugin.EnsureVisible(nodeText);
        }
    }
}
