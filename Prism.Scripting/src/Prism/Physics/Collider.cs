using System;
using System.Numerics;

namespace Prism
{
    public class Collider
    {
        public ulong EntityID { get; protected set; }
        public bool IsTrigger { get; protected set; }
        public Entity Entity { get; protected set; }

        private RigidBodyComponent m_RigidBody;

        protected Collider(ulong entityID)
        {
            EntityID = entityID;
            Entity = new Entity(entityID);
        }

        public RigidBodyComponent RigidBody
        {
            get
            {
                if (m_RigidBody == null)
                    m_RigidBody = Entity.GetComponent<RigidBodyComponent>();

                return m_RigidBody;
            }
        }

        public override string ToString()
        {
            string type = "Collider";

            if (this is BoxCollider) type = "BoxCollider";
            else if (this is SphereCollider) type = "SphereCollider";
            else if (this is CapsuleCollider) type = "CapsuleCollider";
            else if (this is MeshCollider) type = "MeshCollider";

            return "Collider(" + type + ", " + EntityID + ", " + IsTrigger + ")";
        }
    }

    public class BoxCollider : Collider
    {
        public Vector3 Size { get; protected set; }
        public Vector3 Offset { get; protected set; }

        internal BoxCollider(ulong entityID, bool isTrigger, Vector3 size, Vector3 offset) : base(entityID)
        {
            IsTrigger = isTrigger;
            Size = size;
            Offset = offset;
        }
    }

    public class SphereCollider : Collider
    {
        public float Radius { get; protected set; }

        internal SphereCollider(ulong entityID, bool isTrigger, float radius) : base(entityID)
        {
            IsTrigger = isTrigger;
            Radius = radius;
        }
    }

    public class CapsuleCollider : Collider
    {
        public float Radius { get; protected set; }
        public float Height { get; protected set; }

        internal CapsuleCollider(ulong entityID, bool isTrigger, float radius, float height) : base(entityID)
        {
            IsTrigger = isTrigger;
            Radius = radius;
            Height = height;
        }
    }

    public class MeshCollider : Collider
    {
        public Mesh Mesh { get; protected set; }

        internal MeshCollider(ulong entityID, bool isTrigger, IntPtr mesh) : base(entityID)
        {
            IsTrigger = isTrigger;
            Mesh = new Mesh(mesh);
        }
    }
}
