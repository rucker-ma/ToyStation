using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using ToyStation.TSEngine;
using ToyStation.Vulkan;


namespace Editor.Views
{

    public partial class VulkanPage : UserControl
    {
        public VulkanPage()
        {
            InitializeComponent();
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);

        }
    }
    public class VulkanPageControl : VKControl
    {
        public VulkanPageControl()
        {

        }

        protected override void OnRender()
        {

            //var rect = new VKApi.VKRect(200, 30, Convert.ToUInt32(Bounds.Width), Convert.ToUInt32(Bounds.Height));

            var rect = new ToyStation.VkRect2D();
            rect.extent = new ToyStation.VkExtent2D();
            rect.offset = new ToyStation.VkOffset2D();
            rect.offset.x = 200;
            rect.offset.y = 30;
            rect.extent.width = Convert.ToUInt32(Bounds.Width);
            rect.extent.height = Convert.ToUInt32(Bounds.Height);


            //Engine.TSEngine_UpdateSize(rect);
            TEngine.UpdateSize(rect);
        }
    }
}