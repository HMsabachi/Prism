using System.Runtime.CompilerServices;

namespace Prism
{
    public static class Noise
    {
        public static float PerlinNoise(float x, float y)
        {
            unsafe { return InternalCalls.Prism_Noise_PerlinNoise(x, y);}
        }

    }
}
