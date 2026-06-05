using System;
using System.Runtime.InteropServices;

using Rolky.Managed.Interop;

namespace Prism
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastHit
    {
        public ulong EntityID { get; private set; }
        public Vector3 Position { get; private set; }
        public Vector3 Normal { get; private set; }
        public float Distance { get; private set; }
    }

    public static class Physics
    {
        public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit)
        {
            hit = new RaycastHit();
            unsafe
            {
                fixed (RaycastHit* hitPtr = &hit)
                    return InternalCalls.Prism_Physics_Raycast(&origin, &direction, maxDistance, hitPtr);
            }
        }
    }
}
