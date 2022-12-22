using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Editor.Views
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            AvaloniaXamlLoader.Load(this);
            Renderer.DrawFps= true;
            //PlatformImpl.ScalingChanged
            var vulkanPage = this.Get<Editor.Views.VulkanPage>("VulkanPage");
            vulkanPage.SetScaling(PlatformImpl.RenderScaling);
            PlatformImpl.ScalingChanged = scaling => vulkanPage.SetScaling(scaling);
            PlatformImpl.Resized = (size, reason) => vulkanPage.OnResize(size);
        }
    }
}
