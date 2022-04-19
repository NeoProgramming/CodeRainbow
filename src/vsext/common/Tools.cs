using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Text;
using System.Windows.Media;

namespace CodeRainbow
{
    class Tools
    {
        public const uint clrNone = 0xffffffff;

        public static string extractLine(IWpfTextView view, int pos)
        {
            // regardless of the presence or absence of a selection, we need the first line
            ITextSnapshot sn = view.TextSnapshot;
            ITextSnapshotLine sl = sn.GetLineFromPosition(pos);
            string line = sl.GetText();
            return line;
        }

        public static string extractCaretLine(IWpfTextView view, out int pos)
        {
            // regardless of the presence or absence of a selection, we need the first line
            ITextSnapshotLine selectionBegLine = view.Selection.Start.Position.GetContainingLine();
            string line = selectionBegLine.GetText();

            // still need a cursor if it is in this line (cursor position from the beginning of the line)
            int cpos = view.Caret.Position.BufferPosition.Position;
            int spos = selectionBegLine.Start.Position;

            pos = cpos - spos;
            return line;
        }

        public static double luminance(System.Windows.Media.Color clr)
        {
            // 0..255
            return (0.2126*clr.R + 0.7152*clr.G + 0.0722*clr.B);
        }

        public static Color mostContrastTo(Color clr)
        {
            if(luminance(clr) < 127)
                return Color.FromRgb(255, 255, 255);
            return Color.FromRgb(0, 0, 0);
        }

        public static Color Clr(int clr, byte alpha)
        {
            return Color.FromArgb(alpha, (byte)((clr >> 16) & 0xff), (byte)((clr >> 8) & 0xff), (byte)((clr) & 0xff));
        }

        public static Color Clr(uint clr)
        {
            if (clr == clrNone)
                return Colors.Transparent;
            return Color.FromRgb((byte)((clr >> 16) & 0xff), (byte)((clr >> 8) & 0xff), (byte)(clr & 0xff));
        }

        public static uint Rgb(Color clr)
        {
            if (clr == Colors.Transparent)
                return clrNone;
            return ((uint)clr.R << 16) | ((uint)clr.G << 8) | ((uint)clr.B);
        }
    }
}
