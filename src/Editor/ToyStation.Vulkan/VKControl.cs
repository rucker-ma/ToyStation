using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Platform;
using Avalonia.Rendering.SceneGraph;
using Avalonia.Threading;
using ToyStation.TSEngine;

namespace ToyStation.Vulkan
{
    public abstract class VKControl : Control
    {
        protected abstract void OnRender();
        public sealed override void Render(DrawingContext context)
        {
            OnRender();
            base.Render(context);
            Dispatcher.UIThread.InvokeAsync(InvalidateVisual, DispatcherPriority.Render);
        }
        protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
        {
            base.OnDetachedFromVisualTree(e);
        }

        private class VKDrawOperation : ICustomDrawOperation
        {
            public Rect Bounds => throw new NotImplementedException();

            public void Dispose()
            {
                throw new NotImplementedException();
            }

            public bool Equals(ICustomDrawOperation? other)
            {
                throw new NotImplementedException();
            }

            public bool HitTest(Avalonia.Point p)
            {
                throw new NotImplementedException();
            }

            public void Render(IDrawingContextImpl context)
            {
                throw new NotImplementedException();
            }
        }
    }
}
