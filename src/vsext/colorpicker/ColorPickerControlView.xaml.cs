using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls.Primitives;

namespace CustomWPFColorPicker
{
    public partial class ColorPickerControlView : UserControl
    {

        #region Events
        public static readonly RoutedEvent ColorChangedEvent =
            EventManager.RegisterRoutedEvent("ColorChanged", RoutingStrategy.Bubble,
                typeof(RoutedPropertyChangedEventHandler<Color>), typeof(ColorPickerControlView));

        public event RoutedPropertyChangedEventHandler<Color> ColorChanged
        {
            add { AddHandler(ColorChangedEvent, value); }
            remove { RemoveHandler(ColorChangedEvent, value); }
        }

        protected virtual void OnColorChanged(Color oldValue, Color newValue)
        {
            // Debug.WriteLine("OnColorChanged (old=" + oldValue + ", new=" + newValue + ")");
            RoutedPropertyChangedEventArgs<Color> args = new RoutedPropertyChangedEventArgs<Color>(oldValue, newValue);
            args.RoutedEvent = ColorPickerControlView.ColorChangedEvent;
            RaiseEvent(args);
        }
        #endregion

        static Brush _CheckerBrush = CodeRainbow.GuiHelper.CreateCheckerBrush();
        public static Brush CheckerBrush { get { return _CheckerBrush; } }

        public SolidColorBrush CurrentBrush
        {
            get { return (SolidColorBrush)GetValue(CurrentColorProperty); }
            set { SetValue(CurrentColorProperty, value); }
        }

        public static DependencyProperty CurrentColorProperty =
            DependencyProperty.Register("CurrentBrush", typeof(SolidColorBrush), typeof(ColorPickerControlView), new PropertyMetadata(Brushes.Black));
        
        public static RoutedUICommand SelectColorCommand = new RoutedUICommand("SelectColorCommand","SelectColorCommand", typeof(ColorPickerControlView));
        private Window _advancedPickerWindow;
        Color oldColor;

        public ColorPickerControlView()
        {
            DataContext = this;
            InitializeComponent();
            CommandBindings.Add(new CommandBinding(SelectColorCommand, SelectColorCommandExecute));
        }

        private void SelectColorCommandExecute(object sender, ExecutedRoutedEventArgs e)
        {
            Color newColor = (Color)ColorConverter.ConvertFromString(e.Parameter.ToString());
            CurrentBrush = new SolidColorBrush(newColor);
            OnColorChanged(oldColor, newColor);
            oldColor = newColor;
        }

        private static void ShowModal(Window advancedColorWindow)
        {
            advancedColorWindow.Owner = Application.Current.MainWindow;
            advancedColorWindow.ShowDialog();
        }

        void AdvancedPickerPopUpKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
                _advancedPickerWindow.Close();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            popup.IsOpen = false;
            e.Handled = false;
        }

        private void NoneColorsClicked(object sender, RoutedEventArgs e)
        {
            Color newColor = Color.FromArgb(255,255,255,255);
            OnColorChanged(oldColor, newColor);
            oldColor = newColor;
        }

        private void MoreColorsClicked(object sender, RoutedEventArgs e)
        {
            popup.IsOpen = false;
            var advancedColorPickerDialog = new AdvancedColorPickerDialog();
            _advancedPickerWindow = new Window
                                        {
                                            AllowsTransparency = true,
                                            Content = advancedColorPickerDialog,
                                            WindowStyle = WindowStyle.None,
                                            ShowInTaskbar = false,
                                            Background = new SolidColorBrush(Colors.Transparent),
                                            Padding = new Thickness(0),
                                            Margin = new Thickness(0),
                                            WindowState = WindowState.Normal,
                                            WindowStartupLocation = WindowStartupLocation.CenterOwner,
                                            SizeToContent = SizeToContent.WidthAndHeight
                                        };
            _advancedPickerWindow.DragMove();
            _advancedPickerWindow.KeyDown += AdvancedPickerPopUpKeyDown;
            advancedColorPickerDialog.DialogResultEvent += AdvancedColorPickerDialogDialogResultEvent;
            advancedColorPickerDialog.Drag += AdvancedColorPickerDialogDrag;
            ShowModal(_advancedPickerWindow);
        }

        void AdvancedColorPickerDialogDrag(object sender, DragDeltaEventArgs e)
        {
            _advancedPickerWindow.DragMove();
        }

        void AdvancedColorPickerDialogDialogResultEvent(object sender, EventArgs e)
        {
            _advancedPickerWindow.Close();
            var dialogEventArgs = (DialogEventArgs)e;
            if (dialogEventArgs.DialogResult == DialogResult.Cancel)
                return;
            CurrentBrush = dialogEventArgs.SelectedColor;
            Color newColor = dialogEventArgs.SelectedColor.Color;
            OnColorChanged(oldColor, newColor);
            oldColor = newColor;
        }
        public void setColor(Color clr)
        {
            CurrentBrush = new SolidColorBrush(clr);
        }

        public void setRgb(uint clr)
        {
            CurrentBrush = new SolidColorBrush(CodeRainbow.Tools.Clr(clr));
        }
        public Color getColor()
        {
            return CurrentBrush.Color;
        }
        public uint getRgb()
        {
            return CodeRainbow.Tools.Rgb(CurrentBrush.Color);
        }
    }
}