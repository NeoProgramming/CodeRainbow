using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.ComponentModel.Design;
using System.Windows;
using Microsoft.Win32;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.Editor;
using CodeRainbow;
using CodeRainbow.OptionPages;

namespace CodeRainbow
{
    /// <summary>
    /// This is the class that implements the package exposed by this assembly.
    ///
    /// The minimum requirement for a class to be considered a valid package for Visual Studio
    /// is to implement the IVsPackage interface and register itself with the shell.
    /// This package uses the helper classes defined inside the Managed Package Framework (MPF)
    /// to do it: it derives from the Package class that provides the implementation of the 
    /// IVsPackage interface and uses the registration attributes defined in the framework to 
    /// register itself and its components with the shell.
    /// </summary>
    // This attribute tells the PkgDef creation utility (CreatePkgDef.exe) that this class is
    // a package.
    [PackageRegistration(UseManagedResourcesOnly = true)]
    // This attribute is used to register the informations needed to show the this package
    // in the Help/About dialog of Visual Studio.
    [InstalledProductRegistration("#110", "#112", "1.0", IconResourceID = 400)]
    // add these 2 Annotations to execute Initialize() immediately when a project is loaded
    [ProvideAutoLoad(VSConstants.UICONTEXT.SolutionHasSingleProject_string)]
    [ProvideAutoLoad(VSConstants.UICONTEXT.SolutionHasMultipleProjects_string)]

    // This attribute is needed to let the shell know that this package exposes some menus.
    [ProvideMenuResource("Menus.ctmenu", 1)]
    // This attribute registers a tool window exposed by this package.
    [ProvideToolWindow(typeof(TreeToolWindow))]
    [ProvideToolWindow(typeof(InfoToolWindow))]
    [ProvideToolWindow(typeof(MsgsToolWindow))]

    // Options pages
    [ProvideOptionPage(typeof(CROptions), "CodeRainbow", "General Options", 14340, 17770, true)]

    [Guid(GuidList.guidcr_vsextPkgString)]
    public sealed class CRVSExtPackage 
        : Package
        , IVsSolutionEvents3
        , IVsRunningDocTableEvents4//, IVsRunningDocTableEvents3, IVsRunningDocTableEvents2
        , IVsRunningDocTableEvents
    {
        public EnvDTE.DTE _dte;
        private IVsSolution solution = null;
        private uint _hSolutionEvents = uint.MaxValue;

        public CROptions Settings { get; private set; }

        /// <summary>
        /// Default constructor of the package.
        /// Inside this method you can place any initialization code that does not require 
        /// any Visual Studio service because at this point the package object is created but 
        /// not sited yet inside Visual Studio environment. The place to do all the other 
        /// initialization is the Initialize method.
        /// </summary>
        public CRVSExtPackage()
        {
            Trace.WriteLine(string.Format(CultureInfo.CurrentCulture, "Entering constructor for: {0}", this.ToString()));
            CRPlugin.Inst.package = this;
            CRInterop.initCR();            
        }

     //   private EnvDTE.Document CurrentWindow { get { return _dte.ActiveDocument; } }
     //   private EnvDTE.TextSelection CurrentText { get { return (CurrentWindow.Selection as EnvDTE.TextSelection); } }
 
        public void OpenFileAndLine(string path, int line)
        {
            try
            {
                if (path != null)
                    _dte.ItemOperations.OpenFile(path);//, EnvDTE.Constants.vsViewKindTextView);//, Constants.vsViewKindTextView);

                line += 1;
                if (line > 0)
                {
                    EnvDTE.Document doc = _dte.ActiveDocument;
                    if (doc != null && doc.Selection != null)
                    {
                        EnvDTE.TextSelection sel = doc.Selection;
                        sel.GotoLine(line);
                    }
                }
            }
            catch
            {
            }
        }

        public void OpenFileAndMark(string path, string text)
        {
            if (path != null)
                _dte.ItemOperations.OpenFile(path);//, EnvDTE.Constants.vsViewKindTextView);//, Constants.vsViewKindTextView);

            if (text != null && text != "")
            {
                EnvDTE.Document doc = _dte.ActiveDocument;
                if (doc != null && doc.Selection != null)
                {
                    EnvDTE.TextSelection sel = doc.Selection;
                    sel.FindText(text);//, 256 | 8);
                }
            }
        }

        /// <summary>
        /// This function is called when the user clicks the menu item that shows the 
        /// tool window. See the Initialize method to see how the menu item is associated to 
        /// this function using the OleMenuCommandService service and the MenuCommand class.
        /// </summary>
        public void ShowTreeToolWindow(object sender, EventArgs e)
        {
            // Get the instance number 0 of this tool window. This window is single instance so this instance
            // is actually the only one.
            // The last flag is set to true so that if the tool window does not exists it will be created.
            ToolWindowPane window = this.FindToolWindow(typeof(TreeToolWindow), 0, true);
            if ((null == window) || (null == window.Frame))
            {
                throw new NotSupportedException(Resources.CanNotCreateWindow);
            }
            IVsWindowFrame windowFrame = (IVsWindowFrame)window.Frame;
            Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(windowFrame.Show());
        }

        public void ShowInfoToolWindow(object sender, EventArgs e)
        {
            // Get the instance number 0 of this tool window. This window is single instance so this instance
            // is actually the only one.
            // The last flag is set to true so that if the tool window does not exists it will be created.
            ToolWindowPane window = this.FindToolWindow(typeof(InfoToolWindow), 0, true);
            if ((null == window) || (null == window.Frame))
            {
                throw new NotSupportedException(Resources.CanNotCreateWindow);
            }
            IVsWindowFrame windowFrame = (IVsWindowFrame)window.Frame;
            Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(windowFrame.Show());
        }

        public void ShowMsgsToolWindow(object sender, EventArgs e)
        {
            // Get the instance number 0 of this tool window. This window is single instance so this instance
            // is actually the only one.
            // The last flag is set to true so that if the tool window does not exists it will be created.
            ToolWindowPane window = this.FindToolWindow(typeof(MsgsToolWindow), 0, true);
            if ((null == window) || (null == window.Frame))
            {
                throw new NotSupportedException(Resources.CanNotCreateWindow);
            }
            IVsWindowFrame windowFrame = (IVsWindowFrame)window.Frame;
            Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(windowFrame.Show());
        }

        /////////////////////////////////////////////////////////////////////////////
        // Overriden Package Implementation
        #region Package Members

        protected void addMenuItem(OleMenuCommandService mcs, uint id, EventHandler handler)
        {
            CommandID menuCommandID = new CommandID(GuidList.guidcr_vsextCmdSet, (int)id);
            MenuCommand menuItem = new MenuCommand(handler, menuCommandID);
            mcs.AddCommand(menuItem);
        }

        /// <summary>
        /// Initialization of the package; this method is called right after the package is sited, so this is the place
        /// where you can put all the initilaization code that rely on services provided by VisualStudio.
        /// </summary>
        protected override void Initialize()
        {
            Trace.WriteLine (string.Format(CultureInfo.CurrentCulture, "Entering Initialize() of: {0}", this.ToString()));
            base.Initialize();

            this._dte = (EnvDTE.DTE)this.GetService(typeof(EnvDTE.DTE));
            AdviseSolutionEvents();
            Hook(true);

            // Options
            Settings = GetDialogPage(typeof(CROptions)) as CROptions;

            // Add our command handlers for menu (commands must exist in the .vsct file)
            OleMenuCommandService mcs = GetService(typeof(IMenuCommandService)) as OleMenuCommandService;
            if ( null != mcs )
            {
                // Create the command for the menu item.
                addMenuItem(mcs, PkgCmdIDList.cmdidCROpen, onOpenCRFile);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRSave, onSaveCRFile);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRSaveAs, onSaveCRFileAs);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRClear, onClearCR);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRCreateByFolder, onCreateByFolder);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRUpdateByFolder, onUpdateByFolder);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRCreateByProject, onCreateByProject);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRUpdateByProject, onUpdateByProject);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRSplitBase, onSplitBase);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMergeBase, onMergeBase);

                addMenuItem(mcs, PkgCmdIDList.cmdidCRShowTree, ShowTreeToolWindow);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRShowInfo, ShowInfoToolWindow);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRShowOut,  ShowMsgsToolWindow);

                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkArea,  onMarkArea);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkLabel, onMarkLabel);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkBlock, onMarkBlock);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkLsig,  onMarkLsig);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkBsig,  onMarkBsig);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkName,  onMarkName);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkPath,  onMarkPath);

                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkEdit, onMarkEdit);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkLook, onMarkLook);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRMarkTree, onMarkTree);

                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolArea, onMarkArea);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolLabel, onMarkLabel);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolBlock, onMarkBlock);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolLsig, onMarkLsig);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolBsig, onMarkBsig);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolName, onMarkName);

                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolEdit, onMarkEdit);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolLook, onMarkLook);
                addMenuItem(mcs, PkgCmdIDList.cmdidCRToolTree, onMarkTree);
            }
        }

        protected override void Dispose(bool disposing)
        {
            UnadviseSolutionEvents();
            CRInterop.UnloadImportedDll("cr_core.dll");
            base.Dispose(disposing);
        }

        // subscribe to solution events
        private void AdviseSolutionEvents()
        {
            UnadviseSolutionEvents();

            solution = this.GetService(typeof(SVsSolution)) as IVsSolution;

            if (solution != null)
            {
                solution.AdviseSolutionEvents(this, out _hSolutionEvents);
            }
        }

        // unsubscribe from solution events
        private void UnadviseSolutionEvents()
        {
            if (solution != null)
            {
                if (_hSolutionEvents != uint.MaxValue)
                {
                    solution.UnadviseSolutionEvents(_hSolutionEvents);
                    _hSolutionEvents = uint.MaxValue;
                }

                solution = null;
            }
        }

        IVsRunningDocumentTable _docTable;
        bool _hooked;
        uint _cookie;

        private IVsRunningDocumentTable RunningDocumentTable
        {
            [DebuggerStepThrough]
            get { return _docTable ?? (_docTable = GetService(typeof(SVsRunningDocumentTable)) as IVsRunningDocumentTable); }
        }

        // for subscribing to document events
        void Hook(bool enable)
        {
            if (enable == _hooked)
                return;

            IVsRunningDocumentTable rdt = RunningDocumentTable;

            if (rdt == null)
                return;

            if (enable)
            {
                if (ErrorHandler.Succeeded(rdt.AdviseRunningDocTableEvents(this, out _cookie)))
                    _hooked = true;
            }
            else
            {
            //    _docMap.Clear();
            //    _cookieMap.Clear();

                _hooked = false;
                rdt.UnadviseRunningDocTableEvents(_cookie);
            }
        }


        #endregion

        // (1)
        public int OnAfterSaveAll()
        {
            // it is called nevertheless and after closing, and before beforeCloseSolution; ask only if there are changes

            // if necessary, ask if we should save .cr
            if ((CRInterop.getStatus() & 1) == 1) // modify
            {
            //    if (MessageBox.Show("Save changes in CR file?", "CodeRainbow", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                {
                    onSaveCRFile(null, null);
                }
            }

            return VSConstants.S_OK;
        }

        public int OnAfterLastDocumentUnlock(IVsHierarchy pHier, uint itemid, string pszMkDocument, int fClosedWithoutSaving)
        { return VSConstants.S_OK; }
        public int OnBeforeFirstDocumentLock(IVsHierarchy pHier, uint itemid, string pszMkDocument)
        { return VSConstants.S_OK; }

        public int OnAfterAttributeChange(uint docCookie, uint grfAttribs)
        { return VSConstants.S_OK; }
        public int OnAfterDocumentWindowHide(uint docCookie, IVsWindowFrame pFrame)
        { return VSConstants.S_OK; }
        public int OnAfterFirstDocumentLock(uint docCookie, uint dwRDTLockType, uint dwReadLocksRemaining, uint dwEditLocksRemaining)
        { return VSConstants.S_OK; }
        public int OnAfterSave(uint docCookie)
        { return VSConstants.S_OK; }
        public int OnBeforeDocumentWindowShow(uint docCookie, int fFirstShow, IVsWindowFrame pFrame)
        { return VSConstants.S_OK; }
        public int OnBeforeLastDocumentUnlock(uint docCookie, uint dwRDTLockType, uint dwReadLocksRemaining, uint dwEditLocksRemaining)
        { return VSConstants.S_OK; }

        public int OnAfterLoadProject( IVsHierarchy pStubHierarchy, IVsHierarchy pRealHierarchy )
        {
            // Do something
            return VSConstants.S_OK;
        }

        public int OnAfterOpenSolution( object pUnkReserved, int fNewSolution )
        {
            // trying to open *.cr by name *.sln
            CRPlugin.Inst.loadBySLN(_dte.Solution.FullName);
            return VSConstants.S_OK;
        }

        public int OnBeforeUnloadProject( IVsHierarchy pRealHierarchy, IVsHierarchy pStubHierarchy )
        {
            // Do something
            return VSConstants.S_OK;
        }

        // (3)
        public int OnAfterCloseSolution( object pUnkReserved )
        {
            CRPlugin.Inst.ClearTree();
            return VSConstants.S_OK; 
        }

        public int OnAfterClosingChildren( IVsHierarchy pHierarchy )
        { return VSConstants.S_OK; }

        public int OnAfterMergeSolution( object pUnkReserved )
        { return VSConstants.S_OK; }

        public int OnAfterOpenProject( IVsHierarchy pHierarchy, int fAdded )
        { return VSConstants.S_OK; }

        public int OnAfterOpeningChildren( IVsHierarchy pHierarchy )
        { return VSConstants.S_OK; }

        public int OnBeforeCloseProject( IVsHierarchy pHierarchy, int fRemoved )
        { return VSConstants.S_OK; }

        public int OnBeforeClosingChildren( IVsHierarchy pHierarchy )
        { return VSConstants.S_OK; }

        public int OnBeforeOpeningChildren( IVsHierarchy pHierarchy )
        { return VSConstants.S_OK; }

        // (2)
        public int OnBeforeCloseSolution( object pUnkReserved )
        { 
    /*        // if necessary, ask if we should save .cr
            if ((CRInterop.getStatus()&1) == 1)
            {
                if (MessageBox.Show("Save changes in CR file?", "CodeRainbow", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                {
                    onSaveCRFile(null, null);
                }
            }
    */
            return VSConstants.S_OK; 
        }

        public int OnQueryCloseProject( IVsHierarchy pHierarchy, int fRemoving, ref int pfCancel )
        { return VSConstants.S_OK; }

        public int OnQueryCloseSolution( object pUnkReserved, ref int pfCancel )
        { return VSConstants.S_OK; }

        public int OnQueryUnloadProject( IVsHierarchy pRealHierarchy, ref int pfCancel )
        { return VSConstants.S_OK; }


        /// <summary>
        /// This function is the callback used to execute a command when the a menu item is clicked.
        /// See the Initialize method to see how the menu item is associated to this function using
        /// the OleMenuCommandService service and the MenuCommand class.
        /// </summary>
        private void onOpenCRFile(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.FileName = CRInterop.getRecentPath();
            dlg.Filter = "CodeRainbow projects|*.cr";
            dlg.Title = "Select a CodeRainbow Project File";
            if (dlg.ShowDialog() == true)
            {
                if(CRPlugin.Inst.loadCR(dlg.FileName) == false)
                    MessageBox.Show(string.Format("Error opening file {0}", dlg.FileName), "CodeRainbow");
            }
        }

        private void onSaveCRFile(object sender, EventArgs e)
        {
            if (CRInterop.saveCR("") == 0)
                onSaveCRFileAs(sender, e);
        }

        private void onSaveCRFileAs(object sender, EventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();

            string path =  CRInterop.getRecentPath(); 
            if(path == "")
                path = System.IO.Path.ChangeExtension(_dte.Solution.FileName, ".cr");

            dlg.InitialDirectory = System.IO.Path.GetDirectoryName(path);
            dlg.FileName = System.IO.Path.GetFileName(path);
            dlg.Filter = "CodeRainbow projects|*.cr";
            dlg.Title = "Save CodeRainbow Project File";
            if (dlg.ShowDialog() == true)
            {
                if(CRInterop.saveCR(dlg.FileName) == 0)
                    MessageBox.Show(string.Format("Error saving file {0}", dlg.FileName), "CodeRainbow");
            }
        }

        private void onClearCR(object sender, EventArgs e)
        {
            if (MessageBox.Show("Clear CR tree? Tree will be lost!", "CodeRainbow", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
            {
                CRPlugin.Inst.ClearTree();
                CRPlugin.Inst.LoadTree();
            }
        }

        private void onCreateByFolder(object sender, EventArgs e)
        {
            NewProjectDlg dlg = new NewProjectDlg(true);
            if (dlg.ShowDialog() == true)
            {
                uint flags = 0;
                if (CRPlugin.Inst.package.Settings.MakeCRFiles)
                    flags |= 1;
                CRPlugin.Inst.ClearTree();
                CRInterop.createByDir(dlg.ProjName, dlg.ProjBase, dlg.Filters, flags);
                CRPlugin.Inst.LoadTree();
            }
        }

        private void onUpdateByFolder(object sender, EventArgs e)
        {
            NewProjectDlg dlg = new NewProjectDlg(false);
            if (dlg.ShowDialog() == true)
            {
                uint flags = 0;
                if (CRPlugin.Inst.package.Settings.MakeCRFiles)
                    flags |= 1;
                CRInterop.updateByDir(dlg.Filters, flags);
                CRPlugin.Inst.ReloadTree();
            }
        }

        private void onCreateByProject(object sender, EventArgs e)
        {
            int s = CRInterop.getStatus();
            if (((s & 4)!=0) || (MessageBox.Show("Create new CR tree? Existing tree will be lost!", "CodeRainbow", MessageBoxButton.YesNo) == MessageBoxResult.Yes))
            {
                CRPlugin.Inst.ClearTree();
                CRPlugin.Inst.UpdateTreeBySLN(CRInterop.getFiles());
            }
        }

        private void onUpdateByProject(object sender, EventArgs e)
        {
            if (MessageBox.Show("Update new CR tree? Existing tree will be modified", "CodeRainbow", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
            {
                CRPlugin.Inst.UpdateTreeBySLN(CRInterop.getFiles());
            }
        }

        // split the database into files
        private void onSplitBase(object sender, EventArgs e)
        {
            CRInterop.convertBase(1);
        }
        // merge database into one file
        private void onMergeBase(object sender, EventArgs e)
        {
            CRInterop.convertBase(0);
        }

        // context menu handlers
        private void onMarkArea(object sender, EventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(0, 0);
        }
        private void onMarkLabel(object sender, EventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(1, 0);
        }
        private void onMarkBlock(object sender, EventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(2, 0);
        }
        private void onMarkLsig(object sender, EventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(3, 0);
        }
        private void onMarkBsig(object sender, EventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(4, 0);
        }
        private void onMarkName(object sender, EventArgs e)
        {
            CRPlugin.Inst.handleRecentItem(5, 0);
        }

        private void onMarkPath(object sender, EventArgs e)
        {
            // insert current path into FILES
            CRPlugin.Inst.insertThisPath();
        }

        private void onMarkEdit(object sender, EventArgs e)
        {
            CRPlugin.Inst.editMcmt(); 
        }
        private void onMarkLook(object sender, EventArgs e)
        {
            CRPlugin.Inst.lookMcmt();
        }
        private void onMarkTree(object sender, EventArgs e)
        {
            ShowTreeToolWindow(null, null);
            CRPlugin.Inst.lookTree();
        }

        // getting active view (useful thing!)
        // get the active WpfTextView, if there is one.
        public IWpfTextView GetActiveTextView()
        {
            IWpfTextView view = null;
            IVsTextView vTextView;

            var txtMgr = (IVsTextManager)GetService(typeof(SVsTextManager));

            const int mustHaveFocus = 1;
            txtMgr.GetActiveView(mustHaveFocus, null, out vTextView);

            var userData = vTextView as IVsUserData;
            if (null != userData)
            {
                object holder;
                var guidViewHost = DefGuidList.guidIWpfTextViewHost;
                userData.GetData(ref guidViewHost, out holder);
                var viewHost = (IWpfTextViewHost)holder;
                view = viewHost.TextView;
            }
            
            return view;
        }
    }
}
