using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Resources;
//using System.Drawing;
using System.Windows.Media.Imaging;
using System.Windows.Documents;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Text;
using System.Windows.Media;



namespace CodeRainbow
{
    public enum NodeType {
        NT_NONE,        // unknown node
        NT_ROOT,        // main root
        NT_FBASE,       // filesystem root
        NT_TAGS,        // tags root
        NT_OUTLINE,		// outline root

        NT_FILE,        // file
        NT_DIR,         // native directory
        NT_GROUP,       // virtual group
        NT_LINK,        // link to external resource
        NT_TAG,         // tag (multiple marker)

        NT_AREA,        // area mk
        NT_LABEL,       // label mk
        NT_BLOCK,       // block mk
        NT_LSIG,        // label signature
        NT_BSIG,        // block signature
        NT_NAME,        // name signature

        NT_AEND,        // special marker for end of AREA
        NT_ItemsCount
    };

    public struct TextPos
    {
        public int row, col;
    }

    public class CRPlugin
    {
        

        public static TextPos getCaretPos(IWpfTextView textView)
        {
            ITextSnapshotLine selectionBegLine = textView.Selection.Start.Position.GetContainingLine();
            int cpos = textView.Caret.Position.BufferPosition.Position;
            int spos = selectionBegLine.Start.Position;
            TextPos tp;
            tp.row = selectionBegLine.LineNumber;
            tp.col = cpos - spos;
            return tp;
        }

        

        public static int Idx(int t)
        {
            return t - (int)NodeType.NT_AREA;
        }
        public static int Typ(int i)
        {
            return i + (int)NodeType.NT_AREA;
        }

        public Color recentTreeClr;
        public CodeRainbow.TreePane paneTree;
        public CodeRainbow.InfoPane paneInfo;
        public CodeRainbow.MsgsPane paneMsgs;
        public CRVSExtPackage package;
        public List<CRHighlighter> highlighters = new List<CRHighlighter>();

        System.Windows.Media.ImageSource[] icons = new System.Windows.Media.ImageSource[(int)NodeType.NT_ItemsCount];
        
        private static readonly CRPlugin instance = new CRPlugin();
        private CRPlugin()
        {
        //    MessageBox.Show("CRPlugin");
        }
        public static CRPlugin Inst
        {
            get { return instance; }
        }

        public void init()
        {
            initIcons();
        }

        public void clearMessages()
        {
            if (paneMsgs != null)
                paneMsgs.ClearMsgs();
        }

        public void addMessage(string path, int line, string msg)
        {
            if (paneMsgs != null)
                paneMsgs.AddMsg(path, line, msg);
        }

        public void loadInfo(UIntPtr node)
        {
            if(paneInfo != null)
                paneInfo.loadText(node);
        }

        private static TreeViewItem createTreeItem(UIntPtr node, bool chk)
        {
            // creating a visual tree item
            string title = CRInterop.getNodeDisp(node);
            int nt = (int)CRInterop.getNodeType(node);
            uint tclr = CRInterop.getNodeTColor(node);

            TreeViewItem child = new TreeViewItem();
            StackPanel pan = new StackPanel();
            pan.Orientation = Orientation.Horizontal;

            pan.Margin = new Thickness(2, 2, 2, 2);
            
            // pictogram
            System.Windows.Controls.Image image = new System.Windows.Controls.Image();
            image.Height = 16;
            image.Source = Inst.getIcon(nt);
            pan.Children.Add(image);

            // checkbox
            if (chk)
            {
                CheckBox check = new CheckBox();
                check.IsChecked = true;
                check.Checked += Inst.TreeView_Checked;
                check.Unchecked += Inst.TreeView_Unchecked;
                check.Tag = node;
                check.Margin = new Thickness(2, 0, 2, 0);
                pan.Children.Add(check);
            }

            // text
            TextBlock text = new TextBlock(new Run(title));
            pan.Children.Add(text);
            pan.Height += 2;
            child.Header = pan;

            // tag
            child.Tag = node;

            // background color
            if (tclr != Tools.clrNone)
            {
                Color clr = Tools.Clr(tclr);
                text.Background = new SolidColorBrush(clr);
                text.Foreground = new SolidColorBrush(Tools.mostContrastTo(clr));
            }

            // reverse binding - the visual node is stored in the core
            if (chk)
            {
                GCHandle gch = GCHandle.Alloc(child);
                CRInterop.setNodeParam(node, GCHandle.ToIntPtr(gch));
            }
            return child;
        }

        private static TreeViewItem loadTreeLevel(ItemCollection items, UIntPtr node, bool chk, UIntPtr nodeToShow)
        {
            TreeViewItem showItem = null;
            int n = CRInterop.getChildCount(node);
            for (int i = 0; i < n; i++)
            {
                UIntPtr child = CRInterop.getChild(node, i);
                TreeViewItem item = createTreeItem(child, chk);
                items.Add(item);
                if (child == nodeToShow)
                    showItem = item;
                item = loadTreeLevel(item.Items, child, chk, nodeToShow);
                if(item != null)
                    showItem = item;
            }
            return showItem;
        }

        private void TreeView_Checked(object sender, RoutedEventArgs e)
        {
            // checkbox setting handler
            CheckBox checkedcheckbox = e.OriginalSource as CheckBox;
            CRInterop.checkNode((UIntPtr)checkedcheckbox.Tag, 1);
            // refresh active windows
            RefreshAllTextViews(false, false);
        }

        private void TreeView_Unchecked(object sender, RoutedEventArgs e)
        {
            // checkbox uncheck handler
            CheckBox checkedcheckbox = e.OriginalSource as CheckBox;
            CRInterop.checkNode((UIntPtr)checkedcheckbox.Tag, 0);
            // refresh active windows
            RefreshAllTextViews(false, false);
        }

        public static TreeViewItem getTreeItem(UIntPtr node)
        {
            // get visual element by model node
            if (node != UIntPtr.Zero)
            {
                IntPtr handle = CRInterop.getNodeParam(node);
                if (handle != IntPtr.Zero)
                {
                    GCHandle gch = GCHandle.FromIntPtr(handle);
                    TreeViewItem item = (TreeViewItem)gch.Target;
                    return item;
                }
            }
            return null;
        }

        public static void EnsureVisible(UIntPtr node)
        {
            // provide visibility for the main tree
            if (node != UIntPtr.Zero)
            {
                TreeViewItem item = getTreeItem(node);
                if (item != null)
                {
                    CRPlugin.Inst.package.ShowTreeToolWindow(null,null);
                    item.BringIntoView();
                    item.IsSelected = true;
                    item.Focus();
                }
            }
        }

        public static void unlinkTreeItem(TreeViewItem item)
        {
            // detach the associated parameter from the node; this is required before deletion
            if (item.Tag != null)
            {
                UIntPtr node = (UIntPtr)item.Tag;
                item.Tag = null;
                IntPtr param = CRInterop.getNodeParam(node);
                GCHandle gch = GCHandle.FromIntPtr(param);
                gch.Free();
            }

            // and all child elements
            unlinkTreeLevel(item.Items);
        }

        private static void unlinkTreeLevel(ItemCollection items)
        {
            //release tree level
            foreach (TreeViewItem item in items)
            {
                unlinkTreeItem(item);
            }
        }

        private static void unloadTree(TreeView tree)
        {
            // unload the tree; go through all the elements and release the handles
            unlinkTreeLevel(tree.Items);
            tree.Items.Clear();
        }

        public static void loadTree(TreeView tree, bool chk, UIntPtr nodeToShow)
        {
            unloadTree(tree);

            UIntPtr root = CRInterop.getRoot();
            TreeViewItem showItem = loadTreeLevel(tree.Items, root, chk, nodeToShow);
            if (showItem != null)
            {
                showItem.BringIntoView();
                showItem.IsSelected = true;
            }
        }

        public static void setActiveTreeNode(TreeView tree, UIntPtr node, UIntPtr prev)
        {
            if (CRInterop.setActiveNode(node) != 0)
            {
                // remove "bold" from the previous active
                TreeViewItem prevItem = CRPlugin.getTreeItem(prev);
                if (prevItem != null)
                    setItemBold(prevItem, false);

                // put "bold" on current
                TreeViewItem currItem = CRPlugin.getTreeItem(node);
                if (currItem != null)
                    setItemBold(currItem, true);
            }
        }

        private static void setItemBold(TreeViewItem item, bool bold)
        {
            // highlight in bold
            StackPanel pan = (StackPanel)item.Header;
            TextBlock text = (TextBlock)pan.Children[2];
            FontWeight old = text.FontWeight;
            text.FontWeight = bold ? FontWeights.Bold : FontWeights.Normal;
        }



        public static void removeTreeItem(TreeViewItem item)
        {
            // release and remove the node
            unlinkTreeItem(item);
            (item.Parent as TreeViewItem).Items.Remove(item);
        }

        public static void insertTreeItem(UIntPtr node, UIntPtr npar, int index)
        {
            // add and load node
            TreeViewItem par = getTreeItem(npar);
            if (par != null)
            {
                // create a new element
                TreeViewItem item = createTreeItem(node, true);
                if (index >= 0)
                    par.Items.Insert(index, item);
                else
                    par.Items.Add(item);
                // load all children for it
                loadTreeLevel(item.Items, node, true, UIntPtr.Zero);
                item.IsSelected = true;
                par.IsExpanded = true;
                // Attention! refresh is NOT done here, this is a lower-level function and is used, for example, to load a tree
                 // no refresh can be done on loading each element!
            }
        }


        public System.Windows.Media.ImageSource initIcon(string fname)
        {
            string path1 = "pack://application:,,,/cr_vsext;component/vsext/Resources/" + fname;
            return new BitmapImage(new Uri(path1, UriKind.Absolute));
        }

        public void initIcon(NodeType nt, string fname)
        {
            icons[(int)nt] = initIcon(fname);
        }

        public void initIcons()
        {
            initIcon(NodeType.NT_NONE,      "node-node.png");
            initIcon(NodeType.NT_ROOT,      "node-root.png");
            initIcon(NodeType.NT_FBASE,     "node-base.png");
            initIcon(NodeType.NT_TAGS,      "node-tags.png");
            initIcon(NodeType.NT_OUTLINE,   "node-outline.png");

            initIcon(NodeType.NT_FILE,      "node-file.png");
            initIcon(NodeType.NT_DIR,       "node-dir.png");
            initIcon(NodeType.NT_GROUP,     "node-group.png");
            initIcon(NodeType.NT_LINK,      "node-link.png");
            initIcon(NodeType.NT_TAG,       "node-tag.png");

            initIcon(NodeType.NT_AREA,      "node-area.png");
            initIcon(NodeType.NT_LABEL,     "node-label.png");
            initIcon(NodeType.NT_BLOCK,     "node-block.png");
            initIcon(NodeType.NT_LSIG,      "node-lsig.png");
            initIcon(NodeType.NT_BSIG,      "node-bsig.png");
            initIcon(NodeType.NT_NAME,      "node-name.png");

            initIcon(NodeType.NT_AEND,      "misc-noclr.png");
        }

        public System.Windows.Media.ImageSource getIcon(int nt)
        {
            return icons[nt];
        }
        public System.Windows.Media.ImageSource getIcon(NodeType nt)
        {
            return icons[(int)nt];
        }
        public System.Windows.Controls.Image getImage(NodeType nt)
        {
            return getImage((int)nt);
        }

        public System.Windows.Controls.Image getImage(int nt)
        {
            var image = new System.Windows.Controls.Image();
            image.Source = getIcon(nt);
            image.Height = 16;
            return image;
        }

        public System.Windows.Controls.Image makeImage(uint clr)
        {
            var image = new System.Windows.Controls.Image();
            var bitmap = new WriteableBitmap(16, 16, 96, 96, System.Windows.Media.PixelFormats.Bgr32, null);
            try
            {
                // Reserve the back buffer for updates.
                bitmap.Lock();
                unsafe
                {
                    IntPtr pBackBuffer = bitmap.BackBuffer;
                    for (int i = 0; i < 16 * 16; i++)
                    {
                        *((uint*)pBackBuffer + i) = clr;
                    }
                }
                bitmap.AddDirtyRect(new Int32Rect(0, 0, 16, 16));
            }
            finally
            {
                // Release the back buffer and make it available for display.
                bitmap.Unlock();
            }
            image.Height = 16;
            image.Width = 16;
            image.Source = bitmap;
            return image;
        }

        public void loadBySLN(string path)
        {
            string result = System.IO.Path.ChangeExtension(path, ".cr");
            loadCR(result);
        }

        public bool loadCR(string path)
        {
            int res = CRInterop.loadCR(path);
            if (res == 0)
                return false;
            recentTreeClr = Tools.Clr(CRInterop.getNodeTColor(UIntPtr.Zero));
            loadTree(paneTree.treeContent, true, UIntPtr.Zero);
            paneTree.updateRecentMenus();
            return true;
        }

        public void insertThisPath()
        {
            IWpfTextView textView = package.GetActiveTextView();
            if (textView == null)
                return;
            string path = CRPlugin.Inst.GetFileName(textView);
            CRInterop.insertByPath(path);
            ReloadTree();
        }
        
        private UIntPtr lookupNodeByCursor()
        {
            IWpfTextView textView = package.GetActiveTextView();
            if (textView == null)
                return UIntPtr.Zero;
            string text = textView.TextSnapshot.GetText();
            int size = text.Length;
            string path = CRPlugin.Inst.GetFileName(textView);
            UIntPtr fitem = CRInterop.getFileItem(path);
            TextPos pos = getCaretPos(textView);

            UIntPtr node = CRInterop.findNodeByCursor(fitem, text, size, pos.row, pos.col);
            return node;
        }

        public void lookMcmt()
        {
            UIntPtr node1 = lookupNodeByCursor();
            loadInfo(node1);
            CRPlugin.Inst.package.ShowInfoToolWindow(null, null);
        }

        public void lookTree()
        {
            UIntPtr node1 = lookupNodeByCursor();
            CRPlugin.EnsureVisible(node1);
        }

        public void updateTreeItem(UIntPtr node)
        {
            TreeViewItem item = getTreeItem(node);
            StackPanel pan = item.Header as StackPanel;
            TextBlock tb = pan.Children[2] as TextBlock;
            Run run = tb.Inlines.ElementAt(0) as Run;
            run.Text = CRInterop.getNodeDisp(node);
        }

        private bool markupDialog()
        {
            CRMarkerInfo info;
            CRInterop.getMarkerInfo(out info);
            MarkupDlg dlg = new MarkupDlg(info);
            dlg.ShowDialog();
            if (!dlg.DialogResult.HasValue || !dlg.DialogResult.Value)
                return false;
            CRInterop.setMarkerInfo(ref dlg.markerInfo);
            return true;
        }

        // editing (active==0) or adding new nodes
        private void modifyCallback(UIntPtr node, UIntPtr active)
        {
            if (active != UIntPtr.Zero)
            {
                TreeViewItem par = getTreeItem(active);
                TreeViewItem item = createTreeItem(node, true);
                par.Items.Add(item);
                // update views
                CRPlugin.Inst.RefreshAllTextViews(true, true);
            }
            else
            {
                updateTreeItem(node);
            }
        }

        public void editMcmt()
        {
            // view and node
            IWpfTextView textView = package.GetActiveTextView();
            if (textView == null)
                return;
            string filePath = CRPlugin.Inst.GetFileName(textView);
            UIntPtr fileItem = CRInterop.getFileItem(filePath);
            
            // text
            string text = textView.TextSnapshot.GetText();
            // cursor position
            TextPos pos = getCaretPos(textView);
            // trying to find the node we are editing in the tree
            CRHighlightInfo info;
            UIntPtr enode = CRInterop.findNodeInfoByCursor(fileItem, text, text.Length, pos.row, pos.col, out info);
            // if not found - there is nothing there? (although it may be a non-working MC !!)
            if (enode == UIntPtr.Zero)
                return;
            // extract string by position
            string str = Tools.extractLine(textView, info.pos);
            // start editing
            int r = CRInterop.beginEditMarker(filePath, str, enode);
            if (r == 0)
                return;
            if (r == 2 && !markupDialog())
                return;
            
            // finishing editing and updating the tree from callback inside
            CRInterop.endEditMarker(out str, modifyCallback);
            if (str != "")
            {
                // editing a marker comment in a document
                ITextSnapshot snapshot = textView.TextSnapshot;
                using (var edit = snapshot.TextBuffer.CreateEdit())
                {
                    ITextSnapshotLine selectionLine = textView.Selection.Start.Position.GetContainingLine();
                    int posBeg = selectionLine.Start.Position;
                    int posEnd = selectionLine.End.Position;
                    edit.Replace(posBeg, posEnd - posBeg, str); // end is the character AFTER the last
                    edit.Apply();
                }
            }

            // update views
            RefreshAllTextViews(true, false);
            RefreshRecentMenus();
        }

        public void markMcmt(int ri, int ti)
        {
            int t = Typ(ti);
            IWpfTextView textView = package.GetActiveTextView();
            if (textView == null)
                return;
            string filePath = CRPlugin.Inst.GetFileName(textView);
            UIntPtr fileItem = CRInterop.getFileItem(filePath);

            int r, pos;
            string str = Tools.extractCaretLine(textView, out pos);
            r = CRInterop.beginMakeMarker(filePath, ri, t, str, pos);
            if (r == 0)
                return; // something wrong
            if (r == 2 && !markupDialog())
                return; // you need to create a dialog, and it was not created or the user pressed cancel in it
            
            CRMcmtInfo mcmt;
            // finishing editing and updating the tree from casllback inside
            CRInterop.endMakeMarker(out mcmt, modifyCallback);
            // inserting a marker comment into a document
            ITextSnapshot snapshot = textView.TextSnapshot;
            using (var edit = snapshot.TextBuffer.CreateEdit())
            {
                ITextSnapshotLine selectionBegLine = textView.Selection.Start.Position.GetContainingLine();
                ITextSnapshotLine selectionEndLine = textView.Selection.End.Position.GetContainingLine();
                int posBeg = selectionBegLine.Start.Position;
                int posEnd = selectionEndLine.End.Position;
                if (mcmt.end != "")
                    edit.Insert(posEnd, "\r\n" + mcmt.end);
                if (mcmt.beg != "")
                    edit.Insert(posBeg, mcmt.beg + "\r\n");
                edit.Apply();
            }
            
            // update views
            RefreshAllTextViews(true, false);
            RefreshRecentMenus();
        }

        public void handleRecentItem(int ti, int ri)
        {
            // recent item selection handler
            markMcmt(ri, ti);
        }

        public void handleRecentEdit(int ti, int ri)
        {
            // recent item edit handler
            CRRecentInfo info;
            if (CRInterop.getRecentInfo(ti, ri, out info) != 0)
            {
                EditRecentDlg dlg = new EditRecentDlg(info);
                dlg.ShowDialog();
                if(dlg.DialogResult.HasValue && dlg.DialogResult.Value) 
                {
                    CRInterop.setRecentInfo(ti, ri, ref dlg.recentInfo);
                }
            }
        }

        public void handleNewItem(int ti)
        {
            // new item selection handler
            markMcmt(-1, ti);
        }

        

        public string GetFileName(IWpfTextView textView)
        {
            var textDoc = GetDocument(textView);
            if (textDoc != null)
                return textDoc.FilePath;
            else
                return "";
        }

        public ITextDocument GetDocument(IWpfTextView textView)
        {
            var textBuffer = textView.TextBuffer;
            ITextDocument textDoc;
            var rc = textBuffer.Properties.TryGetProperty<ITextDocument>(typeof(ITextDocument), out textDoc);
            if (rc == true)
                return textDoc;
            else
                return null;
        }

        public void ClearTree()
        {
            if (paneTree != null)
                unloadTree(paneTree.treeContent);
            CRInterop.clearCR();
            if (paneTree != null)
                loadTree(paneTree.treeContent, true, UIntPtr.Zero);
        }

        public void LoadTree()
        {
            if (paneTree != null)
                loadTree(paneTree.treeContent, true, UIntPtr.Zero);
        }

        public void ReloadTree()
        {
            // traverse the tree and load new nodes (that don't have a tag) into the display tree
            UIntPtr rnode = CRInterop.getRoot();
            ReloadTreeLevel(rnode);
        }

        private void ReloadTreeLevel(UIntPtr rnode)
        {
            int n = CRInterop.getChildCount(rnode);
            for (int i = 0; i < n; i++)
            {
                UIntPtr cnode = CRInterop.getChild(rnode, i);
                IntPtr handle = CRInterop.getNodeParam(cnode);
                if (handle == IntPtr.Zero)
                {
                    insertTreeItem(cnode, rnode, -1);
                }
                ReloadTreeLevel(cnode);
            }
        }

        public void UpdateTreeBySLN(UIntPtr rnode)
        {
            // hourglass cursor
            System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.WaitCursor;
            // loop by solution
            for (int i = 1; i <= package._dte.Solution.Projects.Count; i++)
            {
                EnvDTE.Project proj = package._dte.Solution.Projects.Item(i);
                
                UIntPtr child = UpdateNode(rnode, NodeType.NT_DIR, proj.Name, "");
                MakeFilesListR(proj.ProjectItems, child);
            }
            // update views
            RefreshAllTextViews(true, true);
            // return cursor
            System.Windows.Forms.Cursor.Current = System.Windows.Forms.Cursors.Default;
        }

        private void MakeFilesListR(EnvDTE.ProjectItems items, UIntPtr rnode)
        {
            for(int j = 1; j <= items.Count; j++) 
            {
                UIntPtr cnode = UIntPtr.Zero;
                EnvDTE.ProjectItem item = items.Item(j);
                string k = item.Kind;
                string n = item.Name;
                
                if (k == "{66A26722-8FB5-11D2-AA7E-00C04F688DDE}")  
                {
                    EnvDTE.Project proj = item.SubProject;// subproject
                    if (proj != null)
                    {
                        cnode = UpdateNode(rnode, NodeType.NT_DIR, n, "");
                        MakeFilesListR(proj.ProjectItems, cnode);
                    }
                    else // free file
                    {
                        // here index from one!!!
                        string p = item.FileCount > 0 ? item.FileNames[1] : "";
                        cnode = UpdateNode(rnode, NodeType.NT_FILE, n, p);
                        //MakeFilesListR(proj.ProjectItems, cnode);
                    }
                }
                else if (k == "{6BB5F8F0-4483-11D3-8BCF-00C04F8EC28C}")  // folder/filter
                {
                    // and here the index from zero!!!
                    string p = item.FileCount > 0 ? item.FileNames[0] : "";
                    cnode = UpdateNode(rnode, NodeType.NT_DIR, n, p);

                    MakeFilesListR(item.ProjectItems, cnode);
                }
                else  // file?
                {
                    string p = item.FileCount > 0 ? item.get_FileNames(0) : "";
                    string ext = System.IO.Path.GetExtension(n);
                    if(ext != ".filters") 
                        cnode = UpdateNode(rnode, NodeType.NT_FILE, n, p);

                    MakeFilesListR(item.ProjectItems, cnode);
                }

                // Attempt to get all of the files
             //   for (short i = 0; i < item.FileCount; i++)
             //   {
             //       String filePath = item.get_FileNames(i);
             //   }

                
            }
        }

        private UIntPtr UpdateNode(UIntPtr npar, NodeType nt, string name, string path)
        {
            UIntPtr node = UIntPtr.Zero;
            int res = CRInterop.updateNode(npar, (int)nt, name, path, out node);
            // depending on whether the node was added or updated - add or update the screen node
            if (res == 1)   // update
            {
                updateTreeItem(node);
            }
            else if (res == 2)  // add
            {
                insertTreeItem(node, npar, -1);
            }
            return node;
        }

        public void RefreshAllTextViews(bool refresh_signatures, bool refresh_fileitems)
        {
            // updating active windows
            // is called after creating/editing the MK, as well as after checking/unchecking nodes in the tree (probably after
            if(refresh_signatures)
                CRInterop.invalidateSignatures();
            foreach (CRHighlighter hl in highlighters)
            {
                if (refresh_fileitems)
                    hl.UpdateFileItem();
                hl.Reparse();
                hl.Refresh();
            }
        }

        public void RefreshRecentMenus()
        {
            if(paneTree != null)
                paneTree.updateRecentMenus();
        }

        public string GetSrcFromEditor(UIntPtr fitem)
        {
            foreach (CRHighlighter hl in highlighters)
            {
                if (hl.fileItem == fitem)
                {
                    string text = hl.textView.TextSnapshot.GetText();
                    return text;
                }
            }
            return null;
        }

        public void scan(UIntPtr root, string sstr, ScanMode smode)
        {
            bool opened = false;
            // add message delegate
            CRInterop.AddMsgCallback cbAddMsg = (string path, int line, string msg) =>
            {
                if ((smode & ScanMode.F_Single) != 0)
                {
                    package.OpenFileAndLine(path, line);
                    opened = true;
                }
                
                if (!opened)
                {
                    addMessage(path, line, msg);
                }
            };

            // source get delegate
            CRInterop.GetSrcCallback cbGetSrc = (string path, UIntPtr fitem, uint mode, string str) =>
            {
                // see if there is path path among open files; if there is, then we take the code from the editor
                // and pass it along with pass-through parameters to scanSrc; and return 1
                // otherwise return 0
                string text = GetSrcFromEditor(fitem);
                if (text != null)
                {
                    int size = text.Length;
                    CRInterop.scanSrc(fitem, mode, str, text, size, cbAddMsg);
                    return 1;
                }
                return 0;
            };

            // if multiple mode - clear messages
            CRPlugin.Inst.clearMessages();

            // call scan
            CRInterop.scanRoot(root, (uint)smode, sstr, cbGetSrc, cbAddMsg);
        }

        public void openNode(UIntPtr node)
        {
            NodeType t = CRInterop.getNodeType(node);
            if (t == NodeType.NT_FILE)
            {
                string path = CRInterop.getNodePath(node);
                CRPlugin.Inst.package.OpenFileAndLine(path, 0);
            }
            else if (t == NodeType.NT_AREA || t == NodeType.NT_LABEL || t == NodeType.NT_BLOCK)
            {
                UIntPtr fnode = CRInterop.getPathNode(node);
            //    string path = CRInterop.getNodePath(fnode);
            //   CRPlugin.Inst.package.OpenFileAndMark(path, CRInterop.getNodeId(node));//CRInterop.makeRegExp(node));
                scan(fnode, CRInterop.getNodeId(node), ScanMode.M_Fuids|ScanMode.F_Single);
            }
            else if (t == NodeType.NT_LSIG || t == NodeType.NT_BSIG || t == NodeType.NT_NAME)
            {
                NodeType st = CRInterop.getNodeSysType(node);
                if (st == NodeType.NT_TAGS)
                {
                    package.ShowMsgsToolWindow(null, null);
                    scan(CRInterop.getRoot(), CRInterop.getNodeId(node), ScanMode.M_Tokens);
                }
                else if (st == NodeType.NT_FBASE)
                {
                    UIntPtr fnode = CRInterop.getPathNode(node);
                    scan(fnode, CRInterop.getNodeId(node), ScanMode.M_Tokens|ScanMode.F_Single);
                }
                else if (st == NodeType.NT_OUTLINE)
                {
                    UIntPtr fnode = CRInterop.getPathNode(node);
                    scan(fnode, CRInterop.getNodeId(node), ScanMode.M_Tokens|ScanMode.F_Single|ScanMode.F_ThisFile);
                }
           //     else
           //     {
           //         UIntPtr fnode = CRInterop.getPathNode(node);
           //         string path = CRInterop.getNodePath(fnode);
            //        CRPlugin.Inst.package.OpenFileAndMark(path, CRInterop.getNodeId(node));//CRInterop.makeRegExp(node));
             //   }
            }
            else if (t == NodeType.NT_TAG)
            {
                package.ShowMsgsToolWindow(null, null);
                scan(CRInterop.getRoot(), CRInterop.getNodeId(node), ScanMode.M_Tags);
            }
        }
    }
}
