using System.Runtime.InteropServices;
using ToyStation.TSEngine;
namespace ToyStation
{
namespace TSEngine
{
[StructLayout(LayoutKind.Sequential)]
public struct VkCtx
{
    public System.IntPtr instance;
    public System.IntPtr physice_device;
    public System.IntPtr device;
    public System.IntPtr queue;
    public uint graphics_queue_idx;
}
}
}
namespace ToyStation
{
namespace TSEngine
{
internal static class VulkanContextGen
{
    [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void VulkanContext_CreateInstance(System.IntPtr func, bool EnableDebug);
    [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void VulkanContext_CreateSurface(System.IntPtr hwnd);
    [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    internal static extern VkCtx VulkanContext_GetContext();
    [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    internal static extern System.IntPtr VulkanContext_GetDeviceProcAddr(string name);
    [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    internal static extern System.IntPtr VulkanContext_GetInstanceProcAddr(string name);

}
public static partial class VulkanContext
{
    public static void CreateInstance(System.IntPtr func, bool EnableDebug)
    {
        VulkanContextGen.VulkanContext_CreateInstance(func, EnableDebug);
    }
    public static void CreateSurface(System.IntPtr hwnd)
    {
        VulkanContextGen.VulkanContext_CreateSurface(hwnd);
    }
    public static VkCtx GetContext()
    {
        return VulkanContextGen.VulkanContext_GetContext();
    }
    public static System.IntPtr GetDeviceProcAddr(string name)
    {
        return VulkanContextGen.VulkanContext_GetDeviceProcAddr(name);
    }
    public static System.IntPtr GetInstanceProcAddr(string name)
    {
        return VulkanContextGen.VulkanContext_GetInstanceProcAddr(name);
    }

}
}
}
