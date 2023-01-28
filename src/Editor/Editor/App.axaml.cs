using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Editor.ViewModels;
using Editor.Views;
using Themes = Avalonia.Themes;
namespace Editor
{
    public partial class App : Application
    {
        public override void Initialize()
        {
            AvaloniaXamlLoader.Load(this);
            
        }

        public override void OnFrameworkInitializationCompleted()
        {
            if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
            {
                desktop.MainWindow = new MainWindow
                {
                    DataContext = new MainWindowViewModel(),
                };
                
                //desktop.Exit += OnExit;
            }
            var theme = new Themes.Default.DefaultTheme();
            theme.TryGetResource("Button", out _);
            
            base.OnFrameworkInitializationCompleted();
        }
        // void OnExit(object sender, ControlledApplicationLifetimeExitEventArgs e)
        // {
        //     if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        //     {
        //         var vm = (PlayerViewModel)desktop.MainWindow?.DataContext;
        //         if (vm != null)
        //             vm.Dispose();
        //     }
        // }
    }
}
