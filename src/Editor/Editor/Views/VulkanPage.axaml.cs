using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using ToyStation.TSEngine;
using ToyStation.Vulkan;


namespace Editor.Views
{
    public partial class VulkanPage : UserControl
    {
        private VulkanPageControl page;
        public VulkanPage()
        {
            AvaloniaXamlLoader.Load(this);
            page = this.Get<VulkanPageControl>("Vulkan");
        }
        public void SetScaling(double scaling)
        {
            page.scaling= scaling;
        }
        public void OnResize(Avalonia.Size size)
        {
            Console.WriteLine("OnResize");
            TEngine.Render().FrameResized();
        }
    }
    public class VulkanPageControl : VKControl
    {
        public double scaling { get; set; }
        public VulkanPageControl()
        {

        }
        
        protected override void OnRender()
        {
            //var rect = new VKApi.VKRect(200, 30, Convert.ToUInt32(Bounds.Width), Convert.ToUInt32(Bounds.Height));
            if(TransformedBounds.HasValue)
            {
                Rect Clip = TransformedBounds.Value.Clip;

                var rect = new ToyStation.VkRect2D();
                rect.extent = new ToyStation.VkExtent2D()
                {
                    width = Convert.ToUInt32(Clip.Width),
                    height= Convert.ToUInt32(Clip.Height)
                };
                rect.offset = new ToyStation.VkOffset2D
                {
                    x = Convert.ToInt32(Clip.X),
                    y = Convert.ToInt32(Clip.Y)
                };
                Dispatcher.UIThread.Post(() =>
                {
                    TEngine.UpdateSize(rect);
                    TEngine.Render().SetScale(scaling);
                });

            }
        }
        
    }
}