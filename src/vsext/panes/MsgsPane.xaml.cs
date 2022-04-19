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
    class MsgViewItem 
    {
        public string Path;
        public int Line;
        public string Location { get; set; }
        public string CodeSample { get; set; }
    }

    /// <summary>
    /// Interaction logic for OutPane.xaml
    /// </summary>
    public partial class MsgsPane : System.Windows.Controls.UserControl
    {
        public MsgsPane()
        {
            InitializeComponent();
            CRPlugin.Inst.paneMsgs = this;
        }

        public void AddMsg(string path, int line, string msg)
        {
            listMessages.Items.Add(new MsgViewItem() { 
                Location = string.Format("{0}:{1}", path, line), 
                CodeSample = msg,
                Path = path,
                Line = line 
            });
        }

        public void ClearMsgs()
        {
            listMessages.Items.Clear();
        }

        private void listMessages_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            var sel = listMessages.SelectedItem as MsgViewItem;
            if(sel != null)
                CRPlugin.Inst.package.OpenFileAndLine(sel.Path, sel.Line);
        }
    }
}
