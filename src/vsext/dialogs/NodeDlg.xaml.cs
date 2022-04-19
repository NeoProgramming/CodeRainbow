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
    /// Interaction logic for NodeDlg.xaml
    /// </summary>
    public partial class NodeDlg : Window
    {
        public CRMarkerInfo nodeInfo;
        public NodeDlg(CRMarkerInfo info)
        {
            InitializeComponent();
            nodeInfo = info;

            comboType.Items.Add("File");
            comboType.Items.Add("Dir");
            comboType.Items.Add("Group");
            comboType.Items.Add("Link");
            comboType.Items.Add("Tag");
            comboType.Items.Add("Area");
            comboType.Items.Add("Label");
            comboType.Items.Add("Block");
            comboType.Items.Add("LSig");
            comboType.Items.Add("BSig");
            comboType.Items.Add("Name");

        //    comboType.SelectedIndex = info.

            textId.Text   = info.id;
            textName.Text = info.name;
            textText.Text = info.text;
            textPath.Text = info.path;
            textInc.Text  = info.incpath;
            ColorNode.setRgb(info.nclr);

            checkMerge.IsChecked = (info.flags & Flags.Merge) != 0;
        }

        private void buttonOk_Click(object sender, RoutedEventArgs e)
        {
            nodeInfo.id   = textId.Text;
            nodeInfo.name = textName.Text;
            nodeInfo.text = textText.Text;
            nodeInfo.path = textPath.Text;
            nodeInfo.nclr = ColorNode.getRgb();
            if (checkMerge.IsChecked == true)
                nodeInfo.flags |= Flags.Merge;
            else
                nodeInfo.flags &= ~Flags.Merge;
            this.DialogResult = true;
            Close();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
           // Close();
        }
    }
}
