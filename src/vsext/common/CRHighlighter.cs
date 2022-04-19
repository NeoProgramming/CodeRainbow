using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Text.Formatting;
using System;
using System.Runtime.InteropServices;
using System.Windows.Shapes;

namespace CodeRainbow
{
    [StructLayout(LayoutKind.Sequential)]
    public struct CRMark
    {
        public int pos;    // position from the beginning of the text
        public int row;    // line number==block number
        public int col;    // column number==character number
        public int lines;  // number of lines; the sign of symbolic selection (and not lowercase) is transmitted in the sign
        public int chars;  // number of bytes from pos to end
        public int color;  // color
    }

	public class CRHighlighter
	{
		private IAdornmentLayer adornmentLayer;
        public IWpfTextView textView;
        private string filePath;
        public UIntPtr fileItem = UIntPtr.Zero;
		private Dictionary<string, FormatInfo> keywordFormats = new Dictionary<string, FormatInfo>();
        bool firstRefresh = true;
        // parsing results
        CRMark[] markersData = null;

		public CRHighlighter(IWpfTextView view)
		{
			textView = view;
            adornmentLayer = view.GetAdornmentLayer("cr_vsext"); // this name must be the same as in Factory [Name("cr_vsext")]

			// Listen to any event that changes the layout (text changes, scrolling, etc)
			textView.LayoutChanged += OnLayoutChanged;
            textView.Closed += OnClosed;
            var textBuffer = textView.TextBuffer;
            if (textBuffer != null)
            {
                textBuffer.Changed += OnTextBufferChanged;
                textBuffer.PostChanged += OnTextBufferPostChanged;
            }

            // get node if possible
            UpdateFileItem();
            
            CRPlugin.Inst.highlighters.Add(this);

        //    CRPlugin.Inst.RefreshAllTextViews(true); // This is not possible. leads to some kind of exception
		}

        public void UpdateFileItem()
        {
            filePath = CRPlugin.Inst.GetFileName(textView);
            fileItem = CRInterop.getFileItem(filePath);
        }

        void RecolorMarkers(IList<ITextViewLine> lines, CRMark[] markers)
        {
            // Grab a reference to the lines in the current TextView
			IWpfTextViewLineCollection textViewLines = textView.TextViewLines;

            foreach (CRMark mark in markers) {
                FormatInfo fi = new FormatInfo(
                    Tools.Clr(mark.color, 128), 
                    Colors.Transparent);
                MakeMarker(textViewLines, mark.pos, mark.pos + mark.chars, mark.lines<0, fi);
            }
        }

        public void Reparse()
        {
            // work with text:
            // moved - now only when entering text, and not on every scroll!

            // get the whole buffer
            string text = textView.TextSnapshot.GetText();
            int size = text.Length;

            // pass to the library and get an array of results
            IntPtr markersReceiver = IntPtr.Zero;
            int n = CRInterop.parseHighlight(text, size, fileItem, out markersReceiver);

            // subtract the result
            markersData = null;
            if (n > 0 && markersReceiver != IntPtr.Zero)
            {
                IntPtr currMarkerReceiver = markersReceiver;
                markersData = new CRMark[n];
                for (int i = 0; i < n; i++)
                {
                    markersData[i] = (CRMark)Marshal.PtrToStructure(currMarkerReceiver, typeof(CRMark));
                    currMarkerReceiver += Marshal.SizeOf(typeof(CRMark));
                }
                //  int[] pIntArray = new int[n];
                //  Marshal.Copy(markersReceiver, pIntArray, 0, n);
                Marshal.FreeCoTaskMem(markersReceiver);
            }
        }

        public void Refresh()
        {
            // complete update of the coloring of the whole window
            this.adornmentLayer.RemoveAllAdornments();
                            
            
            // output coloring
            if (markersData != null) 
            {
                // drawing; here you need to run the comparison algorithm with the previous array
                RecolorMarkers(textView.TextViewLines, markersData);
            }
        }

        // The Changed event is called every time an edit has been made (e.g. typing a char, 
        // pasting code or programmatical changes). It also reacts on programmatical changes.
        private void OnTextBufferChanged(object sender, TextContentChangedEventArgs e)
        {
            // working with text: in theory only when entering text, and not on every scroll!
            Reparse();
        }

        private void OnTextBufferPostChanged(object sender, EventArgs e)
        {
            //todo: try to paste here?
        }

		// On layout change add the adornment to any reformatted lines
		private void OnLayoutChanged(object sender, TextViewLayoutChangedEventArgs args)
		{
            // well, the primary one, when the document has just loaded
            if (firstRefresh == true)
            {
                Reparse();
                firstRefresh = false;
            }

            Refresh();
		}

        private void OnClosed(object sender, EventArgs eventArgs)
        {
            // detach events
            textView.LayoutChanged -= OnLayoutChanged;
            var textBuffer = textView.TextBuffer;
            if (textBuffer != null)
            {
                textBuffer.Changed -= OnTextBufferChanged;
                textBuffer.PostChanged -= OnTextBufferPostChanged;
            }

            CRPlugin.Inst.highlighters.Remove(this);
        }
        
        private void MakeMarker(IWpfTextViewLineCollection textViewLines, int pos1, int pos2, bool sym, FormatInfo kvpval)
        {
            //@@@ range resp. keyword in a line
			SnapshotSpan span = new SnapshotSpan(textView.TextSnapshot, Span.FromBounds(pos1, pos2));
            
			// the geometry of this range on the whole file
            Geometry markerGeometry = textViewLines.GetMarkerGeometry(span);
			if (markerGeometry != null)
			{
                // if it wasn't there
                // I wonder if this is removed, it will be worse?
			//	if (!geometries.Any(g => g.FillContainsWithDetail(markerGeometry) > IntersectionDetail.Empty))
			//	{
                    // add the geometry to the list of geometries (this is just so that there are no overlaps)
			//		geometries.Add(markerGeometry);
                    // and add the marker itself
                if(sym)
                    AddSymMarker(span, markerGeometry, kvpval);
				else
                    AddBarMarker(span, markerGeometry, kvpval);

			//	}
			}
        }

		private void AddSymMarker(SnapshotSpan span, Geometry markerGeometry, FormatInfo formatInfo)
		{
			GeometryDrawing drawing = new GeometryDrawing(formatInfo.Background, formatInfo.Outline, markerGeometry);
			drawing.Freeze();

			DrawingImage drawingImage = new DrawingImage(drawing);
			drawingImage.Freeze();

			Image image = new Image();
			image.Source = drawingImage;

			// Align the image with the top of the bounds of the text geometry
			Canvas.SetLeft(image, markerGeometry.Bounds.Left);
			Canvas.SetTop(image, markerGeometry.Bounds.Top);
                        
            //@@@ essence - adding a layer
			adornmentLayer.AddAdornment(AdornmentPositioningBehavior.TextRelative, span, null, image, null);
		}

        private void AddBarMarker(SnapshotSpan span, Geometry markerGeometry, FormatInfo formatInfo)
        {
            // create a rectangle (and this is a GUI element, not just a rectangle)
            var rect = new Rectangle()
            {
                Height = markerGeometry.Bounds.Height,
                Width = textView.ViewportWidth,
                Fill = formatInfo.Background
            };

            // set top left corner
            Canvas.SetLeft(rect, textView.ViewportLeft);
            Canvas.SetTop(rect, markerGeometry.Bounds.Top);

            adornmentLayer.AddAdornment(span, null, rect);
        }

        // inner class
		// Contains data for a keyword formatting.
        // description of the format of highlighted words (TODO, DEBUG), but the words themselves are not stored here
		private class FormatInfo
		{
			public FormatInfo(Color backgroundColor, Color outlineColor)
			{
				if (backgroundColor.A > 0)
				{
					Background = new SolidColorBrush(backgroundColor);
					Background.Freeze();    // something to optimize
				}
				if (outlineColor.A > 0)
				{
					var penBrush = new SolidColorBrush(outlineColor);
                    penBrush.Freeze();   // something to optimize
					Outline = new Pen(penBrush, 1);
                    Outline.Freeze();      // something to optimize
				}
			}
            // properties
			public Brush Background { get; set; }
			public Pen Outline { get; set; }
		}
	}
}
