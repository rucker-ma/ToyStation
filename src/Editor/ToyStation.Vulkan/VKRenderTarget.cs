using Avalonia.Platform;
using Avalonia.Skia;
using SkiaSharp;
using ToyStation.TSEngine;

namespace ToyStation.Vulkan
{
    internal class VKRenderTarget : ISkiaGpuRenderTarget
    {
        public bool IsCorrupted { get; }
        internal GRContext GRContext { get; set; }

        public bool TickEveryFrame { get; }

        internal uint GraphicsQueueIndex;
        //IPlatformNativeSurfaceHandle SurfaceHandle;
        IPlatformHandle SurfaceHandle;
        public VKRenderTarget(GRContext context, uint Index, IPlatformHandle surface)
        {
            GRContext = context;
            GraphicsQueueIndex = Index;
            TickEveryFrame = true;
            SurfaceHandle = surface;
        }
        public ISkiaGpuRenderSession BeginRenderingSession()
        {
            Render render = TEngine.Render();
            //render.SetScale(SurfaceHandle.Scaling);
            //VKApi.Render_SetScaling(SurfaceHandle.Scaling);
            ToyStation.VkExtent2D extent = new ToyStation.VkExtent2D();

            //extent.width = (uint)SurfaceHandle.Size.Width;
            //extent.height = (uint)SurfaceHandle.Size.Height;

            ImageInfo vKImageInfo = render.GetNextImage(extent);
            //VKApi.VKImageInfo vKImageInfo = VKApi.Render_GetNextImageInfo(new VKApi.VKImageSize(SurfaceHandle.Size.Width,SurfaceHandle.Size.Height));
            lock (GRContext)
            {
                GRContext.ResetContext();
                var imageInfo = new GRVkImageInfo()
                {
                    CurrentQueueFamily = GraphicsQueueIndex,
                    Format = vKImageInfo.format,
                    Image = vKImageInfo.image,
                    ImageLayout = vKImageInfo.layout,
                    ImageTiling = vKImageInfo.tiling,
                    ImageUsageFlags = vKImageInfo.usage_flags,
                    LevelCount = 1,
                    SampleCount = 1,
                    Protected = false,
                    Alloc = new GRVkAlloc()
                    {
                        Memory = vKImageInfo.memory,
                        Flags = 0,
                        Offset = 0,
                        Size = vKImageInfo.memory_size
                    }
                };

                var renderTarget =
                          new GRBackendRenderTarget(vKImageInfo.width, vKImageInfo.height, 1,
                              imageInfo);
                var surface = SKSurface.Create(GRContext, renderTarget,
                   GRSurfaceOrigin.TopLeft,
                    SKColorType.Bgra8888, SKColorSpace.CreateSrgb());

                if (surface == null)
                    throw new InvalidOperationException(
                        $"Surface can't be created with the provided render target");

                return new VKGpuSession(GRContext, renderTarget, surface, SurfaceHandle);
            }
        }

        public void Dispose()
        {
            //todo: clean resource
        }
    }
    internal class VKGpuSession : ISkiaGpuRenderSession
    {
        private readonly GRBackendRenderTarget _gRBackendRenderTarget;
        //private readonly IPlatformNativeSurfaceHandle _nativeSurfaceHandle;
        private readonly IPlatformHandle _nativeSurfaceHandle;
        public VKGpuSession(GRContext context, GRBackendRenderTarget backendRenderTarget, SKSurface surface, IPlatformHandle NativeHandle)
        {
            GrContext = context;
            SkSurface = surface;

            _gRBackendRenderTarget = backendRenderTarget;
            _nativeSurfaceHandle = NativeHandle;

        }
        public GRContext GrContext { get; }

        public SKSurface SkSurface { get; }

        public double ScaleFactor => 1.0;

        public GRSurfaceOrigin SurfaceOrigin => GRSurfaceOrigin.TopLeft;

        public void Dispose()
        {
            SkSurface.Canvas.Flush();
            SkSurface.Dispose();
            _gRBackendRenderTarget.Dispose();
            GrContext.Flush();
            //Engine.TSEngine_Tick();
            TEngine.Tick();
        }
    }
}
