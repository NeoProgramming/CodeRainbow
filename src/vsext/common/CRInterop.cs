using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace CodeRainbow
{
    [Flags]
    public enum Flags : uint
    {
        None = 0,
        Files = 1,
        Outline = 2,
        Tags = 4,
        GenId = 8,
        Inline = 16,
        Merge = 32      // merger of Title+ID
    }

    public enum ScanMode : uint
    {
        M_Coverage,
        M_Tokens,
        M_Fuids,
        M_Tags,
        F_ModeMask  = 0xF,       // 4 bits per mode

        F_Single    = 0x10000,   // look for a single occurrence and immediately open the file
        F_ThisFile  = 0x20000,   // search only in fnode file
    }

    public enum InsertMode : int
    {
        None,
        Parent,
        Predecessor,
        Subsequent,
        Replacement
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    public struct CRRecentInfo
    {
        [MarshalAs(UnmanagedType.BStr)]
        public string name;
        public uint clr;
        public Flags flags;
    }

    // exchange structure for the markup dialog
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    public struct CRMarkerInfo
    {
        [MarshalAs(UnmanagedType.BStr)]
        public string id;
        [MarshalAs(UnmanagedType.BStr)]
        public string tags;
        [MarshalAs(UnmanagedType.BStr)]
        public string name;
        [MarshalAs(UnmanagedType.BStr)]
        public string text;
        [MarshalAs(UnmanagedType.BStr)]
        public string path;
        [MarshalAs(UnmanagedType.BStr)]
        public string incpath;
        public Flags flags;
        public uint nclr;   // from node
        public uint mclr;   // from marker (redefine node clr if need)
        public uint tclr;   // from tree
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    public struct CRMcmtInfo
    {
        [MarshalAs(UnmanagedType.BStr)]
        public string beg;
        [MarshalAs(UnmanagedType.BStr)]
        public string end;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    public struct CRHighlightInfo
    {
        public int pos;    // position from the beginning of the text
        public int row;    // line number==block number
        public int col;    // column number==character number
        public int lines;  // number of lines or characters per line (depending on what kind of marker it is)
        public int chars;  // number of bytes from pos to end
        public uint clr;
    }

    public class CRInterop
    {
        // tree update callback
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void ModifyCallback(UIntPtr node, UIntPtr active);

        // callback to get the code if it's open in the editor; otherwise null
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate int GetSrcCallback([MarshalAs(UnmanagedType.BStr)] string fpath, UIntPtr node, uint mode, [MarshalAs(UnmanagedType.BStr)] string str);

        // callback to add the result to the output window
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void AddMsgCallback([MarshalAs(UnmanagedType.BStr)] string fpath, int line, [MarshalAs(UnmanagedType.BStr)] string msg);

        // unloading
        [DllImport("kernel32", SetLastError = true)]
        private static extern bool FreeLibrary(IntPtr hModule);
        public static void UnloadImportedDll(string dll)
        {
            foreach (System.Diagnostics.ProcessModule mod in System.Diagnostics.Process.GetCurrentProcess().Modules)
            {
                string modname = mod.ModuleName;
                // System.Diagnostics.Debug.WriteLine(mod.ModuleName);
                // if (mod.FileName == dll)
                if (string.Compare(modname, dll, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    FreeLibrary(mod.BaseAddress);
                }
            }
        }


        // If the function name in the method definition matches the entry point name in the DLL,
        // you don't need to explicitly define the function using the EntryPoint field. Otherwise,
        // to specify a name or ordinal, use one of the following attribute forms
        // [DllImport("dllname", EntryPoint="Functionname")]  
        // [DllImport("dllname", EntryPoint = "#123")] 

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void initCR();

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int getStatus();

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getRecentPath();
        
        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern int loadCR([MarshalAs(UnmanagedType.LPWStr)] string fpath);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern int saveCR([MarshalAs(UnmanagedType.LPWStr)] string fpath);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void clearCR();

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr getRoot();

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr getFiles();

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr getTags();

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr getOutline();

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int getChildCount(UIntPtr root);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr getChild(UIntPtr root, int i);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr getPathNode(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getNodeDisp(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getNodeBrief(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getNodeId(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getNodePath(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getNodeDir(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getNodeText(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getNodeFullName(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void setNodeText(UIntPtr node, [MarshalAs(UnmanagedType.LPWStr)] string text);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern NodeType getNodeType(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern NodeType getNodeSysType(UIntPtr node);

        [DllImport("cr_core", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string getRecentItem(int ti, int ri, ref uint clr);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int canRemoveNode(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int removeNode(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int setActiveNode(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr getActiveNode(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int setNodeParam(UIntPtr node, IntPtr param);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern IntPtr getNodeParam(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern UIntPtr getFileItem([MarshalAs(UnmanagedType.LPWStr)] string fpath);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr getParentNode(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int getNodeInfo(UIntPtr node, [Out] out CRMarkerInfo info);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int setNodeInfo(UIntPtr node, [In] ref CRMarkerInfo info);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern int parseHighlight([MarshalAs(UnmanagedType.LPWStr)] string str, int size, 
            UIntPtr fitem, [Out] out IntPtr markers);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int getRecentInfo(int ti, int ri, [Out] out CRRecentInfo info);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int setRecentInfo(int ti, int ri, [In] ref CRRecentInfo info);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int beginMakeMarker([MarshalAs(UnmanagedType.LPWStr)] string fpath, int ri, int t, [MarshalAs(UnmanagedType.LPWStr)] string str, int index);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void endMakeMarker([Out] out CRMcmtInfo info, ModifyCallback fnModify);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int beginEditMarker([MarshalAs(UnmanagedType.LPWStr)] string fpath, [MarshalAs(UnmanagedType.LPWStr)] string str, UIntPtr enode);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void endEditMarker([Out, MarshalAs(UnmanagedType.BStr)] out string str, ModifyCallback fnModify);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void getMarkerInfo([Out] out CRMarkerInfo info);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void setMarkerInfo([In] ref CRMarkerInfo info);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int moveNodeUp(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int moveNodeDown(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int moveNodeLeft(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int moveNodeRight(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int moveNodeTo(UIntPtr node, UIntPtr npar);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern UIntPtr addNode(UIntPtr nodeActive, int type, [In] ref CRMarkerInfo info);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void checkNode(UIntPtr node, int check);
                
        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern int updateNode(UIntPtr par, int type, [MarshalAs(UnmanagedType.LPWStr)] string name, 
            [MarshalAs(UnmanagedType.LPWStr)] string path, out UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern int createByDir([MarshalAs(UnmanagedType.LPWStr)] string name, 
            [MarshalAs(UnmanagedType.LPWStr)] string path, [MarshalAs(UnmanagedType.LPWStr)] string filters, uint flags);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern int updateByDir([MarshalAs(UnmanagedType.LPWStr)] string filters, uint flags);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern int insertByPath([MarshalAs(UnmanagedType.LPWStr)] string fpath);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern void scanRoot(UIntPtr root, uint mode, [MarshalAs(UnmanagedType.LPWStr)] string str, GetSrcCallback fnGetSrc, AddMsgCallback fnAddMsg);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern void scanSrc(UIntPtr fitem, uint mode, [MarshalAs(UnmanagedType.LPWStr)] string str, 
            [MarshalAs(UnmanagedType.LPWStr)] string text, int size, AddMsgCallback fnAddMsg);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern UIntPtr findNodeByCursor(UIntPtr fitem, [MarshalAs(UnmanagedType.LPWStr)] string text, int size, int row, int col);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern UIntPtr findNodeByCpos(UIntPtr fitem, [MarshalAs(UnmanagedType.LPWStr)] string text, int size, int pos);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void invalidateSignatures();

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern UIntPtr findNodeInfoByCursor(UIntPtr fitem, [MarshalAs(UnmanagedType.LPWStr)] string text, int size, int row, int col,
            [Out] out CRHighlightInfo info);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.BStr)]
        unsafe internal static extern string makeRegExp(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern void convertNode(UIntPtr node, [MarshalAs(UnmanagedType.LPWStr)] string path);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        unsafe internal static extern void convertBase(uint flags);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void setNodeTColor(UIntPtr node, uint clr);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern uint getNodeTColor(UIntPtr node);

        [DllImport("cr_core", CallingConvention = CallingConvention.Cdecl)]
        unsafe internal static extern void setActiveNodeMode(int mode);
    }
}

