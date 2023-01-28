using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;

namespace Editor.Views
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            AvaloniaXamlLoader.Load(this);
            Renderer.DrawFps= true;
            
        }

        private void Button_StartPlay(object? sender, RoutedEventArgs e)
        {
            this.FindControl<PlayerView>("RemotePlayer").OpenPlay();
        }

        private void Button_StopPlay(object? sender, RoutedEventArgs e)
        {
            this.FindControl<PlayerView>("RemotePlayer").StopPlay();
        }

        private void Button_StartWebRtc(object? sender, RoutedEventArgs e)
        {
            this.FindControl<WebRtcView>("RemoteWebRtc").OpenPlay();
        }
    }
}
