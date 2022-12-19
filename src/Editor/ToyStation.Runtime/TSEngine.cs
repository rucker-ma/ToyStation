using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
