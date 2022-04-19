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
    /// Interaction logic for NewProjectDlg.xaml
    /// </summary>
    public partial class NewProjectDlg : Window
    {
        public string ProjName, ProjBase, Filters;
        public NewProjectDlg(bool en)
        {
            InitializeComponent();
            textTitle.Text = ProjName;
            textBase.Text = ProjBase;
            textFilters.Text = Filters;
            textTitle.IsEnabled = en;
            textBase.IsEnabled = en;
        }

        private void buttonOk_Click(object sender, RoutedEventArgs e)
        {
            ProjName = textTitle.Text;
            ProjBase = textBase.Text;
            Filters = textFilters.Text;
            
            this.DialogResult = true;
            Close();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
           // Close();
        }

        private void buttonBrowse_Click(object sender, RoutedEventArgs e)
        {
            using (var fbd = new System.Windows.Forms.FolderBrowserDialog())
            {
                System.Windows.Forms.DialogResult result = fbd.ShowDialog();

                if (result == System.Windows.Forms.DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    textBase.Text = fbd.SelectedPath;
                }
            }
        }
    }
}
