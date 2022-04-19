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
    /// Interaction logic for TreeDlg.xaml
    /// </summary>
    public partial class TreeDlg : Window
    {
        public UIntPtr SelectedNode = UIntPtr.Zero;

        public TreeDlg(UIntPtr nodeToShow)
        {
            InitializeComponent();
            CRPlugin.loadTree(treeView, false, nodeToShow);
            treeView.Focus();
        }

        private void buttonOk_Click(object sender, RoutedEventArgs e)
        {
            TreeViewItem treeViewItem = (TreeViewItem)treeView.SelectedItem;
            SelectedNode = (UIntPtr)treeViewItem.Tag;
            this.DialogResult = true;
            Close();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
           // Close();
        }

        private void buttonActive_Click(object sender, RoutedEventArgs e)
        {
            // set selected node active
            // first remove the activity from the node on which it is
            TreeViewItem treeViewItem = (TreeViewItem)treeView.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;
            UIntPtr prev = CRInterop.getActiveNode(node);

            // set
            CRPlugin.setActiveTreeNode(treeView, node, prev);
            CRPlugin.setActiveTreeNode(CRPlugin.Inst.paneTree.getTree(), node, prev);
        }
    }
}
