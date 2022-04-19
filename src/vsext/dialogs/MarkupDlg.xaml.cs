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
    /// Interaction logic for MarkupDlg.xaml
    /// </summary>
    public partial class MarkupDlg : Window
    {
        public CRMarkerInfo markerInfo;
        uint clrMark;
        uint clrNode;
        UIntPtr nodePar;

        public MarkupDlg(CRMarkerInfo info)
        {
            InitializeComponent();
            //nodePar = CRInterop.getActiveNode(
            markerInfo = info;

            textId.Text   = info.id;
            textTags.Text = info.tags;
            textName.Text = info.name;
            textText.Text = info.text;
            
            checkGenID.IsChecked = (info.flags & Flags.GenId) != 0;
            checkFiles.IsChecked = (info.flags & Flags.Files) != 0;
            checkTags.IsChecked = (info.flags & Flags.Tags) != 0;
            checkOutline.IsChecked = (info.flags & Flags.Outline) != 0;
            checkMerge.IsChecked = (info.flags & Flags.Merge) != 0;

            bool mv = (markerInfo.mclr != Tools.clrNone);
            checkBoxInline.IsChecked = mv;
            clrMark = info.mclr;
            clrNode = info.nclr;
            colorPickerMark.setRgb(mv ? clrMark : clrNode);

            textId.Focus();
        }

        private void buttonOk_Click(object sender, RoutedEventArgs e)
        {
            markerInfo.id   = textId.Text;
            markerInfo.tags = textTags.Text;
            markerInfo.name = textName.Text;
            markerInfo.text = textText.Text;

            markerInfo.flags = 0;
            if (checkGenID.IsChecked == true)
                markerInfo.flags |= Flags.GenId;
            if (checkFiles.IsChecked == true)
                markerInfo.flags |= Flags.Files;
            if (checkTags.IsChecked == true)
                markerInfo.flags |= Flags.Tags;
            if (checkOutline.IsChecked == true)
                markerInfo.flags |= Flags.Outline;
            if (checkMerge.IsChecked == true)
                markerInfo.flags |= Flags.Merge;

            bool mv = checkBoxInline.IsChecked==true;
            markerInfo.mclr = (mv && clrMark != Tools.clrNone) ? clrMark : Tools.clrNone;
            markerInfo.nclr = colorPickerMark.getRgb();

            this.DialogResult = true;
            Close();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
           // Close();
        }

        private void colorPickerMark_ColorChanged(object sender, RoutedPropertyChangedEventArgs<Color> e)
        {
        //    ColorRecent.setColor(CRInterop.clrNone);
        //    ColorPredef.setColor(CRInterop.clrNone); // crash!
            bool chk = checkBoxInline.IsChecked == true;
            if (chk)
                clrMark = colorPickerMark.getRgb();
            else
                clrNode = colorPickerMark.getRgb();
        }

        private void colorPickerTree_ColorChanged(object sender, RoutedPropertyChangedEventArgs<Color> e)
        {
        }

        private void ColorRecent_ColorChanged(object sender, RoutedPropertyChangedEventArgs<Color> e)
        {
        //    ColorPicker.setColor(CRInterop.clrNone);
        //    ColorPredef.setColor(CRInterop.clrNone); // crash!
        }

        private void ColorPredef_ColorChanged(object sender, RoutedPropertyChangedEventArgs<Color> e)
        {
        //    ColorPicker.setColor(CRInterop.clrNone); // crash!
        //    ColorRecent.setColor(CRInterop.clrNone); // crash!
        }

        private void checkBoxInline_Checked(object sender, RoutedEventArgs e)
        {
            bool mv = checkBoxInline.IsChecked == true;
            if (mv)
                colorPickerMark.setRgb(clrMark);
            else
                colorPickerMark.setRgb(clrNode);
        }

        private void buttonTree_Click(object sender, RoutedEventArgs e)
        {
            TreeDlg dlg = new TreeDlg(UIntPtr.Zero);
            dlg.ShowDialog();
            if (dlg.DialogResult.HasValue && dlg.DialogResult.Value)
            {
                textLoc.Text = CRInterop.getNodeFullName(dlg.SelectedNode);
                textLoc.CaretIndex = textLoc.Text.Length;
                var rect = textLoc.GetRectFromCharacterIndex(textLoc.CaretIndex);
                textLoc.ScrollToHorizontalOffset(rect.Right);
                //textLoc.ScrollToEnd();
            }
        }
    }
}
