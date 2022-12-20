using System.Runtime.InteropServices;
using ToyStation.TSEngine;
namespace ToyStation
{
[StructLayout(LayoutKind.Sequential)]
public struct VkExtent2D
{
    public uint width;
    public uint height;
}
}
namespace ToyStation
{
namespace TSEngine
{
[StructLayout(LayoutKind.Sequential)]
public struct ImageInfo
{
    public ulong image;
    public ulong image_view;
    public ulong memory;
    public int width;
    public int height;
    public uint layout;
    public uint tiling;
    public uint format;
    public uint usage_flags;
    public ulong memory_size;
}
}
}
namespace ToyStation
{
namespace TSEngine
{
internal static class RenderGen
{
    [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    internal static extern ImageInfo Render_GetNextImage(System.IntPtr Self, VkExtent2D Size);
    [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void Render_SetScale(System.IntPtr Self, double Scale);

}
public partial class Render
{
    private System.IntPtr _obj;
    public Render(System.IntPtr Ptr)
    {
        _obj = Ptr;
    }
    public ImageInfo GetNextImage(VkExtent2D Size)
    {
        return RenderGen.Render_GetNextImage(_obj, Size);
    }
    public void SetScale(double Scale)
    {
        RenderGen.Render_SetScale(_obj, Scale);
    }

}
}
}
