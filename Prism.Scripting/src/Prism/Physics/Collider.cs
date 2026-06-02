using System;
using System.Numerics;

namespace Prism
{
    public class Collider
    {
        public enum ColliderType
        {
            Box,
            Sphere,
            Capsule,
            Mesh
        }

        public Entity ColliderEntity { get; protected set; }
        public ColliderType Type { get; protected set; }
        public bool IsTrigger { get; protected set; }
    }

    public class BoxCollider : Collider
    {
        public Vector3 Size { get; private set; }
        public Vector3 Offset { get; private set; }

        internal BoxCollider(ulong entityID, Vector3 size, Vector3 offset, bool isTrigger)
        {
            ColliderEntity = new Entity(entityID);
            Type = ColliderType.Box;
            Size = size;
            Offset = offset;
            IsTrigger = isTrigger;
        }
    }

    public class SphereCollider : Collider
    {
        public float Radius { get; private set; }

        internal SphereCollider(ulong entityID, float radius, bool isTrigger)
        {
            ColliderEntity = new Entity(entityID);
            Type = ColliderType.Sphere;
            Radius = radius;
            IsTrigger = isTrigger;
        }
    }

    public class CapsuleCollider : Collider
    {
        public float Radius { get; private set; }
        public float Height { get; private set; }

        internal CapsuleCollider(ulong entityID, float radius, float height, bool isTrigger)
        {
            ColliderEntity = new Entity(entityID);
            Type = ColliderType.Capsule;
            Radius = radius;
            Height = height;
            IsTrigger = isTrigger;
        }
    }

    public class MeshCollider : Collider
    {
        public Mesh Mesh { get; private set; }

        internal MeshCollider(ulong entityID, Mesh mesh, bool isTrigger)
        {
            ColliderEntity = new Entity(entityID);
            Type = ColliderType.Mesh;
            Mesh = mesh;
            IsTrigger = isTrigger;
        }
    }
}
