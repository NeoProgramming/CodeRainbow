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
    /// Interaction logic for EditRecentDlg.xaml
    /// </summary>
    public partial class EditRecentDlg : Window
    {
        public CRRecentInfo recentInfo;
        public EditRecentDlg(CRRecentInfo info)
        {
            InitializeComponent();
            recentInfo = info;

            textId.Text = info.name;
            checkGenId.IsChecked = (info.flags & Flags.GenId) != 0;
            checkFiles.IsChecked = (info.flags & Flags.Files) != 0;
            checkTags.IsChecked = (info.flags & Flags.Tags) != 0;
            checkOutline.IsChecked = (info.flags & Flags.Outline) != 0;
            colorPicker.setRgb(info.clr);
        }

        private void buttonOk_Click(object sender, RoutedEventArgs e)
        {
            recentInfo.name = textId.Text;
            recentInfo.clr = colorPicker.getRgb();
            recentInfo.flags = 0;
            if (checkGenId.IsChecked == true)
                recentInfo.flags |= Flags.GenId;
            if (checkFiles.IsChecked == true)
                recentInfo.flags |= Flags.Files;
            if (checkTags.IsChecked == true)
                recentInfo.flags |= Flags.Tags;
            if (checkOutline.IsChecked == true)
                recentInfo.flags |= Flags.Outline;
            this.DialogResult = true;
            Close();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
           // Close();
        }
    }
}
