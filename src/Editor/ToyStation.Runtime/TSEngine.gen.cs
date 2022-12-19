using System.Runtime.InteropServices;
namespace ToyStation
{
    [StructLayout(LayoutKind.Sequential)]
    public struct VkOffset2D
    {
        public int x;
        public int y;
    }
}
namespace ToyStation
{
    [StructLayout(LayoutKind.Sequential)]
    public struct VkRect2D
    {
        public VkOffset2D offset;
        public VkExtent2D extent;
    }
}
namespace ToyStation
{
    namespace TSEngine
    {
        internal static class TEngineGen
        {
            [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
            internal static extern void TEngine_Init();
            [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
            internal static extern void TEngine_Tick();
            [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
            internal static extern System.IntPtr TEngine_IRender();
            [DllImport("TSEngine", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
            internal static extern void TEngine_UpdateSize(VkRect2D Size);

        }
        public static partial class TEngine
        {
            public static void Init()
            {
                TEngineGen.TEngine_Init();
            }
            public static void Tick()
            {
                TEngineGen.TEngine_Tick();
            }
            public static System.IntPtr IRender()
            {
                return TEngineGen.TEngine_IRender();
            }
            public static void UpdateSize(VkRect2D Size)
            {
                TEngineGen.TEngine_UpdateSize(Size);
            }

        }
    }
}
