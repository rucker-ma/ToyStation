using System.Runtime.InteropServices;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Logging;
using Avalonia.Platform;
using Avalonia.Skia;
using Avalonia.Win32;
using SkiaSharp;

using ToyStation.TSEngine;

namespace ToyStation.Vulkan
{
    public class VKSkiaGpu : ISkiaGpu
    {
        public GRContext GrContext { get; set; }
        private GRVkBackendContext grVKBackend;
        private bool _initialized;
        private uint _graphicsQueueIndex;

        public delegate void VKDebugInfoDelegate(string message);

        public static void PrintVKMessage(string message)
        {
            Logger.TryGet(LogEventLevel.Warning, "Vulkan")?.Log(null, message);
        }

        private VKDebugInfoDelegate del;
        VKSkiaGpu()
        {
            del = new VKDebugInfoDelegate(PrintVKMessage);
            VulkanContext.CreateInstance(Marshal.GetFunctionPointerForDelegate(del), true);
            //VKApi.VK_CreateInstance(Marshal.GetFunctionPointerForDelegate(del));
        }
        public ISkiaGpuRenderTarget TryCreateRenderTarget(IEnumerable<object> surfaces)
        {
            foreach (var surface in surfaces)
            {
                //IPlatformNativeSurfaceHandle

                if (surface is IPlatformHandle handle)
                {
                    VulkanContext.CreateSurface(handle.Handle);
                    //VKApi.VK_CreateSurface(handle.Handle);
                    Initialize();
                    var RenderTarget = new VKRenderTarget(GrContext, _graphicsQueueIndex, handle);
                    return RenderTarget;
                }
            }
            return null;
        }

        public ISkiaSurface TryCreateSurface(PixelSize size, ISkiaGpuRenderSession session)
        {

            Console.WriteLine("TryCreateSurface");

            return null;
        }

        private void Initialize()
        {
            if (_initialized)
                return;
            _initialized = true;

            GRVkGetProcedureAddressDelegate getProcedureDelegate = (name, instanceHandle, deviceHandle) =>
            {
                IntPtr address;

                if (deviceHandle != IntPtr.Zero)
                {
                    address = VulkanContext.GetDeviceProcAddr(name);
                    //address = VKApi.VK_GetDeviceProcAddr(name);
                    if (address != IntPtr.Zero)
                        return address;
                }
                address = VulkanContext.GetInstanceProcAddr(name);
                //address = VKApi.VK_GetInstanceProcAddr(name);
                return address;
            };
            VkCtx vKContext = VulkanContext.GetContext();
            //VKApi.VKContext vKContext = VKApi.VK_GetContext();
            grVKBackend = new GRVkBackendContext
            {
                VkInstance = vKContext.instance,
                VkPhysicalDevice = vKContext.physice_device,
                VkDevice = vKContext.device,
                VkQueue = vKContext.queue,
                GraphicsQueueIndex = vKContext.graphics_queue_idx,
                GetProcedureAddress = getProcedureDelegate
            };
            _graphicsQueueIndex = vKContext.graphics_queue_idx;
            GrContext = GRContext.CreateVulkan(grVKBackend);

            TEngine.Init();
            //Engine.TSEngine_Init();
        }

        public static ISkiaGpu CreateGpu()
        {
            var gpu = new VKSkiaGpu();
            AvaloniaLocator.CurrentMutable.Bind<VKSkiaGpu>().ToConstant(gpu);
            return gpu;
        }
    }

    public static class VulkanAppExtensions
    {
        public static T UseVulkan<T>(this T builder) where T : AppBuilderBase<T>, new()
        {
            return builder.UseSkia()
                .With(new SkiaOptions()
                {
                    CustomGpuFactory = VKSkiaGpu.CreateGpu
                });
        }
    }
}
