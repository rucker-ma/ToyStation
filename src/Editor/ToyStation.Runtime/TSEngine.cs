
namespace ToyStation
{
    namespace TSEngine
    {
        public static partial class TEngine
        {
            public static Render Render()
            {
                return new Render(TEngineGen.TEngine_IRender());
            }
        }
    }
}
