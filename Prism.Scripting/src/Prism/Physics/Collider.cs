using System;
using System.Numerics;

namespace Prism
{
    public enum ColliderType
    {
        Box = 0,
        Sphere = 1,
        Capsule = 2,
        Mesh = 3
    }

    public class Collider
    {
        public ColliderType Type { get; protected set; }
        public ulong EntityID { get; protected set; }
        public bool IsTrigger { get; protected set; }

        public bool Is<T>() where T : Collider
        {
            if (typeof(T) == typeof(BoxCollider) && Type == ColliderType.Box)
            {
                return true;
            }
            else if (typeof(T) == typeof(SphereCollider) && Type == ColliderType.Sphere)
            {
                return true;
            }
            else if (typeof(T) == typeof(CapsuleCollider) && Type == ColliderType.Capsule)
            {
                return true;
            }

            return false;
        }
    }

    public class BoxCollider : Collider
    {
        public Vector3 Size { get; protected set; }
        public Vector3 Offset { get; protected set; }

        internal BoxCollider(ulong entityID, bool isTrigger, Vector3 size, Vector3 offset)
        {
            Type = ColliderType.Box;
            EntityID = entityID;
            IsTrigger = isTrigger;
            Size = size;
            Offset = offset;
        }
    }

    public class SphereCollider : Collider
    {
        public float Radius { get; protected set; }

        internal SphereCollider(ulong entityID, bool isTrigger, float radius)
        {
            Type = ColliderType.Sphere;
            EntityID = entityID;
            IsTrigger = isTrigger;
            Radius = radius;
        }
    }

    public class CapsuleCollider : Collider
    {
        public float Radius { get; protected set; }
        public float Height { get; protected set; }

        internal CapsuleCollider(ulong entityID, bool isTrigger, float radius, float height)
        {
            Type = ColliderType.Capsule;
            EntityID = entityID;
            IsTrigger = isTrigger;
            Radius = radius;
            Height = height;
        }
    }

    public class MeshCollider : Collider
    {
        public Mesh Mesh { get; protected set; }

        internal MeshCollider(ulong entityID, bool isTrigger, string filepath)
        {
            EntityID = entityID;
            IsTrigger = isTrigger;
            Mesh = new Mesh(filepath);
        }
    }
}
