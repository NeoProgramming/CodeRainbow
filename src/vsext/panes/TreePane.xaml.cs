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
using Microsoft.Win32;



namespace CodeRainbow
{
    /// <summary>
    /// Interaction logic for TreePane.xaml
    /// </summary>
    public partial class TreePane : System.Windows.Controls.UserControl
    {
        // data
        TreeViewItem activeItem;

        public TreePane()
        {
        //    MessageBox.Show("TreePane");

            InitializeComponent();
            CRPlugin.Inst.paneTree = this;
            CRPlugin.Inst.init();
            
            CRPlugin.loadTree(treeContent, true, UIntPtr.Zero);
            updateRecentMenus();
        }

        public TreeView getTree()
        {
            return treeContent;
        }

        // Using Command to handle events
        public static RoutedUICommand SplitButtonMainCommand0 = new RoutedUICommand("Split Button 0", "SplitButtonMainCommand0", typeof(TreePane));
        public static RoutedUICommand SplitButtonMainCommand1 = new RoutedUICommand("Split Button 1", "SplitButtonMainCommand1", typeof(TreePane));
        public static RoutedUICommand SplitButtonMainCommand2 = new RoutedUICommand("Split Button 2", "SplitButtonMainCommand2", typeof(TreePane));
        public static RoutedUICommand SplitButtonMainCommand3 = new RoutedUICommand("Split Button 3", "SplitButtonMainCommand3", typeof(TreePane));
        public static RoutedUICommand SplitButtonMainCommand4 = new RoutedUICommand("Split Button 4", "SplitButtonMainCommand4", typeof(TreePane));
        public static RoutedUICommand SplitButtonMainCommand5 = new RoutedUICommand("Split Button 5", "SplitButtonMainCommand5", typeof(TreePane));

        private void Tree_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            TreeViewItem SelectedItem = treeContent.SelectedItem as TreeViewItem;
            // determine what we will have a context menu
            bool sys = false;
            if (SelectedItem != null && SelectedItem.Tag != null)
            {
                UIntPtr tag = (UIntPtr)SelectedItem.Tag;
                NodeType t = CRInterop.getNodeType(tag);
                if (t == NodeType.NT_ROOT || t == NodeType.NT_FBASE || t == NodeType.NT_TAGS || t == NodeType.NT_OUTLINE)
                    sys = true;
            }

            if(sys)
                treeContent.ContextMenu = treeContent.Resources["ctxSys"] as System.Windows.Controls.ContextMenu;
            else
                treeContent.ContextMenu = treeContent.Resources["ctxNode"] as System.Windows.Controls.ContextMenu;
           
            // upload to dashboard
            loadAnnotation(SelectedItem);
        }

        private void loadAnnotation(TreeViewItem SelectedItem)
        {
            // upload to dashboard
            if (SelectedItem != null)
                CRPlugin.Inst.loadInfo((UIntPtr)SelectedItem.Tag);
            else
                CRPlugin.Inst.loadInfo(UIntPtr.Zero);
        }

        private void setItemColor(TreeViewItem item, Color clr)
        {
            // set the background color
            StackPanel pan = (StackPanel)item.Header;
            TextBlock text = (TextBlock)pan.Children[2];
            if (clr.A == 0)
            {
                text.ClearValue(TextBlock.BackgroundProperty);
                text.ClearValue(TextBlock.ForegroundProperty);
            }
            else
            {
                text.Background = new SolidColorBrush(clr);
                text.Foreground = new SolidColorBrush(Tools.mostContrastTo(clr));
            }
        }
        
        private void onNodeSetActive(object sender, RoutedEventArgs e)
        {
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;
            UIntPtr prev = CRInterop.getActiveNode(node);

            CRPlugin.setActiveTreeNode(treeContent, node, prev);
        }

        private void updateMenu(int mode)
        {
            ContextMenu m = treeContent.Resources["ctxNode"] as System.Windows.Controls.ContextMenu;
            foreach (Object obj in m.Items)
            {
                MenuItem item = obj as MenuItem;
                if (item != null && item.Name == "itemSetCurrAs")
                {
                    foreach (Object obj2 in item.Items)
                    {
                        MenuItem item2 = obj2 as MenuItem;
                        if (item2 != null)
                        {
                            if (item2.Name == "itemSetCurrAsPar")
                                item2.IsChecked = (mode == 0);
                            if (item2.Name == "itemSetCurrAsPrev")
                                item2.IsChecked = (mode == 1);
                            if (item2.Name == "itemSetCurrAsNext")
                                item2.IsChecked = (mode == 2);
                            if (item2.Name == "itemSetCurrAsRepl")
                                item2.IsChecked = (mode == 3);
                        }
                    }
                    if (mode == 0)
                        item.Header = "Set current as: Parent";
                    else if (mode == 1)
                        item.Header = "Set current as: Predecessor";
                    else if (mode == 2)
                        item.Header = "Set current as: Subsequent";
                    else if (mode == 3)
                        item.Header = "Set current as: Replacement";
                    return;
                }
            }
        }

        private void onNodeSetAsPar(object sender, RoutedEventArgs e)
        {
            onNodeSetActive(sender, e);
            updateMenu(0);
            CRInterop.setActiveNodeMode(0);
        }
        private void onNodeSetAsPrev(object sender, RoutedEventArgs e)
        {
            onNodeSetActive(sender, e);
            updateMenu(1);
            CRInterop.setActiveNodeMode(1);
        }
        private void onNodeSetAsNext(object sender, RoutedEventArgs e)
        {
            onNodeSetActive(sender, e);
            updateMenu(2);
            CRInterop.setActiveNodeMode(2);
        }
        private void onNodeSetAsRepl(object sender, RoutedEventArgs e)
        {
            onNodeSetActive(sender, e);
            updateMenu(3);
            CRInterop.setActiveNodeMode(3);
        }

        private void onOpenFile(object sender, RoutedEventArgs e)
        {
            // open the file containing the given node
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;
            UIntPtr pnode = CRInterop.getPathNode(node);
            if (pnode == UIntPtr.Zero)
                return;
            string path = CRInterop.getNodePath(pnode);
            if (path == null)
                return;
            CRPlugin.Inst.package.OpenFileAndLine(path, 0);
        }

        private void onNodeFindAll(object sender, RoutedEventArgs e)
        {
        }

        private void onNodeFindInFile(object sender, RoutedEventArgs e)
        {
        }

        private void onNodeEdit(object sender, RoutedEventArgs e)
        {
            // edit handler
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            CRMarkerInfo info = new CRMarkerInfo();
            if (CRInterop.getNodeInfo(node, out info) != 0)
            {
                NodeDlg dlg = new NodeDlg(info);
                dlg.ShowDialog();
                if (dlg.DialogResult.HasValue && dlg.DialogResult.Value)
                {
                    CRInterop.setNodeInfo(node, ref dlg.nodeInfo);
                    CRPlugin.Inst.updateTreeItem(node);
                    CRPlugin.Inst.RefreshAllTextViews(true, false);
                }
            }
        }

        private void onNodeMoveTo(object sender, RoutedEventArgs e)
        {
            // element move handler
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            TreeDlg dlg = new TreeDlg(node);
            dlg.ShowDialog();
            if (dlg.DialogResult.HasValue && dlg.DialogResult.Value)
            {
                if (CRInterop.moveNodeTo(node, dlg.SelectedNode) != 0)
                {
                    // there we do unload the entire subtree and delete
                    CRPlugin.removeTreeItem(treeViewItem);
                    // insert element and load all children
                    CRPlugin.insertTreeItem(node, dlg.SelectedNode, -1);
                }
            }
        }

        private void onNodeAddNode(object sender, RoutedEventArgs e)
        {
            // add an arbitrary node of an arbitrary type
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            CRMarkerInfo info = new CRMarkerInfo();
            NodeDlg dlg = new NodeDlg(info);
            dlg.ShowDialog();
            if (dlg.DialogResult.HasValue && dlg.DialogResult.Value)
            {
                UIntPtr cnode = CRInterop.addNode(node, 0, ref info);
                if (cnode != UIntPtr.Zero)
                {
                    CRPlugin.insertTreeItem(cnode, node, -1);
                    // update views
                    CRPlugin.Inst.RefreshAllTextViews(true, true);
                }
            }
        }
        
        private void onNodeAddGroup(object sender, RoutedEventArgs e)
        {
            // add a group
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            InputDlg dlg = new InputDlg("test");
            dlg.ShowDialog();
            if (dlg.DialogResult.HasValue && dlg.DialogResult.Value)
            {
                CRMarkerInfo info = new CRMarkerInfo();
                info.id = dlg.inputInfo;
                UIntPtr cnode = CRInterop.addNode(node, (int)NodeType.NT_GROUP, ref info);
                if (cnode != UIntPtr.Zero)
                {
                    CRPlugin.insertTreeItem(cnode, node, -1);
                    // update views
                    CRPlugin.Inst.RefreshAllTextViews(true, true);
                }
            }
        }

        private void onNodeAddFile(object sender, RoutedEventArgs e)
        {
            // add an existing file
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "Files|*.*";
            dlg.Title = "Select a File";
            dlg.InitialDirectory = CRInterop.getNodeDir(node);
            if (dlg.ShowDialog() == true)
            {
                CRMarkerInfo info = new CRMarkerInfo();
                info.id = dlg.SafeFileName;
                info.path = dlg.FileName;
                if (CRPlugin.Inst.package.Settings.MakeCRFiles)
                    info.incpath = dlg.FileName + ".cr";
                UIntPtr cnode = CRInterop.addNode(node, (int)NodeType.NT_FILE, ref info);
                if (cnode != UIntPtr.Zero)
                {
                    CRPlugin.insertTreeItem(cnode, node, -1);
                    // update views
                    CRPlugin.Inst.RefreshAllTextViews(true, true);
                }
            }
        }

        private void onNodeAddPath(object sender, RoutedEventArgs e)
        {
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            //CRInterop.updateByDir(
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "Files|*.*";
            dlg.Title = "Select a File";
            //dlg.InitialDirectory = CRInterop.getNodeDir(node);
            if (dlg.ShowDialog() == true)
            {
                CRInterop.insertByPath(dlg.FileName);
                CRPlugin.Inst.ReloadTree();
            }
        }

        private void onNodeAddLink(object sender, RoutedEventArgs e)
        {
            // add link
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            LinkDlg dlg = new LinkDlg("test");
            dlg.ShowDialog();
            if (dlg.DialogResult.HasValue && dlg.DialogResult.Value)
            {
                CRMarkerInfo info = new CRMarkerInfo();
                info.id = dlg.linkInfo;
                UIntPtr cnode = CRInterop.addNode(node, (int)NodeType.NT_LINK, ref info);
                if (cnode != UIntPtr.Zero)
                {
                    CRPlugin.insertTreeItem(cnode, node, -1);
                    // update views
                    CRPlugin.Inst.RefreshAllTextViews(true, true);
                }
            }
        }

        private void onNodeClearColor(object sender, RoutedEventArgs e)
        {
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;
            setItemColor(treeViewItem, new Color());
            CRInterop.setNodeTColor(node, Tools.clrNone);
        }

        private void onNodeSetColor(object sender, RoutedEventArgs e)
        {
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            System.Windows.Forms.ColorDialog dlg2 = new System.Windows.Forms.ColorDialog();
            dlg2.FullOpen = true;
            uint rgb = CRInterop.getNodeTColor(node);
            Color clr = rgb!=Tools.clrNone ? Tools.Clr(rgb) : CRPlugin.Inst.recentTreeClr;
            dlg2.Color = System.Drawing.Color.FromArgb(clr.A, clr.R, clr.G, clr.B);
            if(dlg2.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                clr = Color.FromArgb(dlg2.Color.A, dlg2.Color.R, dlg2.Color.G, dlg2.Color.B);
                setItemColor(treeViewItem, clr);
                CRInterop.setNodeTColor(node, Tools.Rgb(clr));
                CRPlugin.Inst.recentTreeClr = clr;
            }
        }

        private void onNodeSetRecentColor(object sender, RoutedEventArgs e)
        {
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            setItemColor(treeViewItem, CRPlugin.Inst.recentTreeClr);
            CRInterop.setNodeTColor(node, Tools.Rgb(CRPlugin.Inst.recentTreeClr));
        }

        private void onNodeAddNote(object sender, RoutedEventArgs e)
        {
            // add a note -- a note is built into any node by default, is it needed at all??? easier to add group
       /*@     TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            InputDlg dlg = new InputDlg("note");
            dlg.ShowDialog();
            if (dlg.DialogResult.HasValue && dlg.DialogResult.Value)
            {
                CRMarkerInfo info = new CRMarkerInfo();
                info.id = dlg.inputInfo;
                UIntPtr cnode = CRInterop.addNode(node, (int)NodeType.NT_NOTE, ref info);
                if (cnode != UIntPtr.Zero)
                {
                    CRPlugin.insertTreeItem(cnode, node, -1);
                    // update views
                    CRPlugin.Inst.RefreshAllTextViews(true, true);
                }
            }
        */
        }

        private void onNodeLookup(object sender, RoutedEventArgs e)
        {
            // searching for an element and displaying it in the panel - this is already done by selecting the element
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
        //    UIntPtr node = (UIntPtr)treeViewItem.Tag;
        //    CRPlugin.Inst.loadInfo(node);

            // switch to the bookmark forcibly, show it if it does not already exist
            CRPlugin.Inst.package.ShowInfoToolWindow(null, null);
            loadAnnotation(treeViewItem);
        }

        // open the annotation for the current node in the tree
        private void onClickToolsAnn(object sender, RoutedEventArgs e)
        {
            CRPlugin.Inst.package.ShowInfoToolWindow(null, null);
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            loadAnnotation(treeViewItem);
        }


        private void onNodeSeparate(object sender, RoutedEventArgs e)
        {
            // set the selection flag to a separate file
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Filter = "CR Files|*.cr";
            dlg.Title = "Select a File";
            string dir = CRInterop.getNodeDir(node).Replace('/', '\\');
            dlg.InitialDirectory = dir;
            if (dlg.ShowDialog() == true)
            {
                CRInterop.convertNode(node, dlg.FileName);
            }
        }

        private void onNodeRemove(object sender, RoutedEventArgs e)
        {
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            if (treeViewItem == activeItem)
                activeItem = null;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;

            MessageBoxResult dialogResult
                = MessageBox.Show("Are you sure?", "Delete node", MessageBoxButton.YesNo);
            if (dialogResult == MessageBoxResult.Yes)
            {
                // trying to delete: check at the core level
                if (CRInterop.canRemoveNode(node) != 0)
                {
                    // if we can, we first release, because after removal, the release does not work - there is a call to the core
                    CRPlugin.unlinkTreeItem(treeViewItem);
                    // now we are trying to remove from the core
                    if (CRInterop.removeNode(node) != 0)
                    {
                        // if it works, remove it from the screen tree
                        CRPlugin.removeTreeItem(treeViewItem);
                        // update views
                        CRPlugin.Inst.RefreshAllTextViews(true, true);
                    }
                    else
                    {
                        // failed: this is an error, because verification was successful but deletion was not
                        MessageBox.Show("ERROR removing!", "Delete node");
                    }
                }
            }
        }

        private void onNodeInfo(object sender, RoutedEventArgs e)
        {
            TreeViewItem treeViewItem = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)treeViewItem.Tag;
            string info = CRInterop.getNodeBrief(node);
            if (info == null)
                info = "null";
            MessageBox.Show(info, "Node info");
        }

        // dynamic menu click handler
        private void DynamicMenuItemClick(object sender, RoutedEventArgs e)
        {
            var mi = e.OriginalSource as MenuItem;
            if (mi != null)
            {
                int i = 0;
                if(Int32.TryParse(mi.Tag.ToString(), out i))
                {
                    if (i < 100)
                        CRPlugin.Inst.handleRecentItem(i / 10, i % 10);
                    else if (i < 200)
                        CRPlugin.Inst.handleRecentEdit((i - 100) / 10, (i - 100) % 10);
                    else 
                        CRPlugin.Inst.handleNewItem(i-200);
                }
            }
        }

        public void updateRecentMenus()
        {
            updateRecentMenu(0, mySplitBtn0.DropDownContextMenu.Items);
            updateRecentMenu(1, mySplitBtn1.DropDownContextMenu.Items);
            updateRecentMenu(2, mySplitBtn2.DropDownContextMenu.Items);
            updateRecentMenu(3, mySplitBtn3.DropDownContextMenu.Items);
            updateRecentMenu(4, mySplitBtn4.DropDownContextMenu.Items);
            updateRecentMenu(5, mySplitBtn5.DropDownContextMenu.Items);
        }

        void updateRecentMenu(int ti, ItemCollection items)
        {
            // items: [0]=new, [1]=edit, [2]=separator, [3]..[12]=items
            for(int ri=0; ri<10; ri++)
            {
                uint clr = 0xffffffff;
                string text = CRInterop.getRecentItem(ti, ri, ref clr);
                MenuItem item = items[3+ri] as MenuItem;
                if (text == "")
                {
                    item.Header = "<Empty>";
                    item.IsEnabled = false;
                }
                else if (clr == 0xffffffff)
                {
                    item.Header = text;
                    item.Icon = CRPlugin.Inst.getImage(NodeType.NT_AEND); 
                    item.IsEnabled = true;
                }
                else
                {
                    item.Header = text;
                    item.Icon = CRPlugin.Inst.makeImage(clr);
                    item.IsEnabled = true;
                }
            }
        }
        
        //public void removeLocicalChild(object obj)
        //{
        //    // WTF?
        //    RemoveLogicalChild(obj);
       // }

        private void onClickMoveUp(object sender, RoutedEventArgs e)
        {
            // move node up
            TreeViewItem item = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)item.Tag;

            if (CRInterop.moveNodeUp(node) != 0)
            {
                UIntPtr npar = CRInterop.getParentNode(node);
                TreeViewItem par = CRPlugin.getTreeItem(npar);
                int index = par.Items.IndexOf(item);
                CRPlugin.removeTreeItem(item);
                CRPlugin.insertTreeItem(node, npar, index - 1);
            }
        }

        private void onClickMoveDown(object sender, RoutedEventArgs e)
        {
            // move node down
            TreeViewItem item = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)item.Tag;

            if (CRInterop.moveNodeDown(node) != 0)
            {
                UIntPtr npar = CRInterop.getParentNode(node);
                TreeViewItem par = CRPlugin.getTreeItem(npar);
                int index = par.Items.IndexOf(item);
                CRPlugin.removeTreeItem(item);
                CRPlugin.insertTreeItem(node, npar, index + 1);
            }
        }

        private void onClickMoveLeft(object sender, RoutedEventArgs e)
        {
            // move node to the left
            TreeViewItem item = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)item.Tag;
            UIntPtr npar = CRInterop.getParentNode(node);
            UIntPtr nppar = CRInterop.getParentNode(npar);
            
            if (CRInterop.moveNodeLeft(node) != 0)
            {
                TreeViewItem par = CRPlugin.getTreeItem(npar);
                TreeViewItem ppar = CRPlugin.getTreeItem(nppar);
                int index = ppar.Items.IndexOf(par);

                CRPlugin.removeTreeItem(item);
                CRPlugin.insertTreeItem(node, nppar, index + 1);
            }
        }

        private void onClickMoveRight(object sender, RoutedEventArgs e)
        {
            // move node to the right
            TreeViewItem item = (TreeViewItem)treeContent.SelectedItem;
            UIntPtr node = (UIntPtr)item.Tag;
            UIntPtr npar = CRInterop.getParentNode(node);

            if (CRInterop.moveNodeRight(node) != 0)
            {
                TreeViewItem par = CRPlugin.getTreeItem(npar);
                int index = par.Items.IndexOf(item);
                TreeViewItem prev_item = (TreeViewItem)par.Items.GetItemAt(index - 1);

                CRPlugin.removeTreeItem(item);
                CRPlugin.insertTreeItem(node, (UIntPtr)prev_item.Tag, -1);
            }
        }

        

        // run a coverage check
        private void onClickToolsCoverage(object sender, RoutedEventArgs e)
        {
            CRPlugin.Inst.scan(CRInterop.getRoot(), "", ScanMode.M_Coverage);
        }

        // trigger editing of the marker comment under the cursor
        private void onClickToolsEdit(object sender, RoutedEventArgs e)
        {
        //    CRPlugin.Inst.markMcmt(-2, 0);
            CRPlugin.Inst.editMcmt();
        }

        // load information on the marker comment under the cursor in the information panel
        private void onClickToolsLookup(object sender, RoutedEventArgs e)
        {
            CRPlugin.Inst.lookMcmt();
        }

        // open the selected node in the tree
        private void onClickToolsTree(object sender, RoutedEventArgs e)
        {
            CRPlugin.Inst.lookTree();
        }

        
        // button click handlers
        private void SplitButtonMainExecuted_Button0(object sender, ExecutedRoutedEventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(0, 0);
        }

        private void SplitButtonMainExecuted_Button1(object sender, ExecutedRoutedEventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(1, 0);
        }

        private void SplitButtonMainExecuted_Button2(object sender, ExecutedRoutedEventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(2, 0);
        }

        private void SplitButtonMainExecuted_Button3(object sender, ExecutedRoutedEventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(3, 0);
        }

        private void SplitButtonMainExecuted_Button4(object sender, ExecutedRoutedEventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(4, 0);
        }

        private void SplitButtonMainExecuted_Button5(object sender, ExecutedRoutedEventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(5, 0);
        }

        static T VisualUpwardSearch<T>(DependencyObject source) where T : DependencyObject
        {
            DependencyObject returnVal = source;

            while (returnVal != null && !(returnVal is T))
            {
                DependencyObject tempReturnVal = null;
                if (returnVal is Visual || returnVal is System.Windows.Media.Media3D.Visual3D)
                {
                    tempReturnVal = VisualTreeHelper.GetParent(returnVal);
                }
                if (tempReturnVal == null)
                {
                    returnVal = LogicalTreeHelper.GetParent(returnVal);
                }
                else returnVal = tempReturnVal;
            }

            return returnVal as T;
        }

        private void OnPreviewMouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            
            TreeViewItem treeViewItem = VisualUpwardSearch<TreeViewItem>(e.OriginalSource as DependencyObject);
            if (treeViewItem != null)
            {
                treeViewItem.Focus();
                treeViewItem.IsSelected = true;
                e.Handled = true;
            }
             
            /*
            TreeViewItem item = sender as TreeViewItem;
            if (item != null)
            {
                item.Focus();
                item.IsSelected = true;
                e.Handled = true;
            }*/
        }

        
        private void OnItemMouseDoubleClick(object sender, MouseButtonEventArgs args)
        {/*@
            // double click
            if (sender is TreeViewItem)
            {
                TreeViewItem item = (TreeViewItem)treeContent.SelectedItem;
                if (item != null)
                {
                    UIntPtr node = (UIntPtr)item.Tag;
                    CRPlugin.Inst.openNode(node);
                }
            }
            //.... do stuff.
          */
            args.Handled = true;
        }
        

        // mouse double click event - like recommended
        // https://stackoverflow.com/questions/6037883/how-to-disable-double-click-behaviour-in-a-wpf-treeview
        private void TreeViewItem_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs args)
        {
            if (args.ClickCount > 1)
            {
                //here you would probably want to include code that is called by your
                //mouse down event handler.
                args.Handled = true;

                // call my handler
                TreeViewItem item = (TreeViewItem)treeContent.SelectedItem;
                if (item != null)
                {
                    UIntPtr node = (UIntPtr)item.Tag;
                    CRPlugin.Inst.openNode(node);
                }
            }
        }
    }
}