using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls.Primitives;

namespace CodeRainbow 
{
    public class GuiHelper
    {
        public static Brush CreateCheckerBrush()
        {
            // from http://msdn.microsoft.com/en-us/library/aa970904.aspx

            DrawingBrush checkerBrush = new DrawingBrush();

            GeometryDrawing backgroundSquare =
                new GeometryDrawing(
                    Brushes.White,
                    null,
                    new RectangleGeometry(new Rect(0, 0, 8, 8)));

            GeometryGroup aGeometryGroup = new GeometryGroup();
            aGeometryGroup.Children.Add(new RectangleGeometry(new Rect(0, 0, 4, 4)));
            aGeometryGroup.Children.Add(new RectangleGeometry(new Rect(4, 4, 4, 4)));

            GeometryDrawing checkers = new GeometryDrawing(Brushes.Black, null, aGeometryGroup);

            DrawingGroup checkersDrawingGroup = new DrawingGroup();
            checkersDrawingGroup.Children.Add(backgroundSquare);
            checkersDrawingGroup.Children.Add(checkers);

            checkerBrush.Drawing = checkersDrawingGroup;
            checkerBrush.Viewport = new Rect(0, 0, 0.5, 0.5);
            checkerBrush.TileMode = TileMode.Tile;

            return checkerBrush;
        }
    }
}
