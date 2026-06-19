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

    [StructLayout(LayoutKind.Sequential)]
    public struct OverlapHitData
    {
        public ulong EntityID;
        public uint ColliderType;   // 0=Box, 1=Sphere, 2=Capsule, 3=Mesh
        public Bool32 IsTrigger;
        public float ShapeData0;
        public float ShapeData1;
        public float ShapeData2;
        public float ShapeData3;
        public float ShapeData4;
        public float ShapeData5;
        public IntPtr MeshHandle;
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

        public static Collider[] OverlapBox(Vector3 origin, Vector3 halfSize)
        {
            NativeArray<OverlapHitData> results;
            unsafe
            {
                InternalCalls.Prism_Physics_OverlapBox(&origin, &halfSize, &results);
            }
            var colliders = new Collider[results.Length];
            for (int i = 0; i < results.Length; i++)
                colliders[i] = CreateColliderFromHitData(results[i]);
            results.Dispose();
            return colliders;
        }

        public static Collider[] OverlapSphere(Vector3 origin, float radius)
        {
            NativeArray<OverlapHitData> results;
            unsafe
            {
                InternalCalls.Prism_Physics_OverlapSphere(&origin, radius, &results);
            }
            var colliders = new Collider[results.Length];
            for (int i = 0; i < results.Length; i++)
                colliders[i] = CreateColliderFromHitData(results[i]);
            results.Dispose();
            return colliders;
        }

        private static Collider CreateColliderFromHitData(OverlapHitData data)
        {
            switch (data.ColliderType)
            {
                case 0: // Box
                    return new BoxCollider(data.EntityID, data.IsTrigger,
                        new Vector3(data.ShapeData0, data.ShapeData1, data.ShapeData2),
                        new Vector3(data.ShapeData3, data.ShapeData4, data.ShapeData5));
                case 1: // Sphere
                    return new SphereCollider(data.EntityID, data.IsTrigger, data.ShapeData0);
                case 2: // Capsule
                    return new CapsuleCollider(data.EntityID, data.IsTrigger, data.ShapeData0, data.ShapeData1);
                case 3: // Mesh
                    return new MeshCollider(data.EntityID, data.IsTrigger, data.MeshHandle);
                default:
                    return null;
            }
        }
    }
}
