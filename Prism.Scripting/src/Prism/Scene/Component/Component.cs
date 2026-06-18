using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Prism
{
#pragma warning disable CS8618
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }
#pragma warning restore CS8618

    public class TagComponent : Component
    {
        public string Tag
        {
            get
            {
                return GetTag_Native(Entity.ID);
            }
            set
            {
                SetTag_Native(value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetTag_Native(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTag_Native(string tag);

    }

    public class MeshRendererComponent : Component
    {
        public Mesh Mesh
        {
            get
            {
                unsafe { return new Mesh(InternalCalls.Prism_MeshRendererComponent_GetMesh(Entity.ID)); }
            }
            set
            {
                IntPtr ptr = value == null ? IntPtr.Zero : value.m_UnmanagedInstance;
                unsafe { InternalCalls.Prism_MeshRendererComponent_SetMesh(Entity.ID, ptr); }
            }
        }

        // TODO: Step 2 — InternalCall 到 C++ 端 Materials[]
        public Material GetMaterial(int index)
        {
            return Material.DefaultMaterial;
        }

        public void SetMaterial(int index, Material material)
        {
            // TODO
        }

        public int GetMaterialCount()
        {
            return 0;
        }
    }

    public class CameraComponent : Component
    {
        // TODO
    }

    public class SpriteRendererComponent : Component
    {
        // TODO
    }

    public class RigidBody2DComponent : Component
    {
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 offset, bool wake)
        {
            unsafe { InternalCalls.Prism_RigidBody2DComponent_ApplyLinearImpulse(Entity.ID, &impulse, &offset, wake); }
        }
        public Vector2 LinearVelocity
        {
            get { return GetLinearVelocity(); }
            set { SetLinearVelocity(value); }
        }
        public Vector2 GetLinearVelocity()
        {
            Vector2 velocity;
            unsafe { InternalCalls.Prism_RigidBody2DComponent_GetLinearVelocity(Entity.ID, &velocity); }
            return velocity;
        }
        public void SetLinearVelocity(Vector2 velocity)
        {
            unsafe { InternalCalls.Prism_RigidBody2DComponent_SetLinearVelocity(Entity.ID, &velocity); }
        }
    }

    public class BoxCollider2DComponent : Component
    {
    }

    public class CircleCollider2DComponent : Component
    {
    }

    public enum ForceMode
    {
        Force = 0,
        Impulse,
        VelocityChange,
        Acceleration
    }

    public class RigidBodyComponent : Component
    {
        public void AddForce(Vector3 force, ForceMode forceMode = ForceMode.Force)
        {
            unsafe { InternalCalls.Prism_RigidBodyComponent_AddForce(Entity.ID, &force, (int)forceMode); }
        }

        public void AddTorque(Vector3 torque, ForceMode forceMode = ForceMode.Force)
        {
            unsafe { InternalCalls.Prism_RigidBodyComponent_AddTorque(Entity.ID, &torque, (int)forceMode); }
        }

        public Vector3 LinearVelocity
        {
            get { return GetLinearVelocity(); }
            set { SetLinearVelocity(value); }
        }

        public Vector3 GetLinearVelocity()
        {
            Vector3 velocity;
            unsafe { InternalCalls.Prism_RigidBodyComponent_GetLinearVelocity(Entity.ID, &velocity); }
            return velocity;
        }

        public void SetLinearVelocity(Vector3 velocity)
        {
            unsafe { InternalCalls.Prism_RigidBodyComponent_SetLinearVelocity(Entity.ID, &velocity); }
        }

        public void Rotate(Vector3 rotation)
        {
            unsafe { InternalCalls.Prism_RigidBodyComponent_Rotate(Entity.ID, &rotation); }
        }

        public float Mass
        {
            get { unsafe { return InternalCalls.Prism_RigidBodyComponent_GetMass(Entity.ID); } }
            set { unsafe { InternalCalls.Prism_RigidBodyComponent_SetMass(Entity.ID, value); } }
        }

        public uint Layer
        {
            get { unsafe { return InternalCalls.Prism_RigidBodyComponent_GetLayer(Entity.ID); } }
        }
    }

    public class BoxColliderComponent : Component
    {
    }

    public class SphereColliderComponent : Component
    {
    }

    public class CapsuleColliderComponent : Component
    {
    }
}
