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
    public abstract class Component
    {
        public Entity Entity { get; set; }

    }

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

    public class MeshComponent : Component
    {
        public Mesh Mesh
        {
            get
            {
                Mesh result = new Mesh(GetMesh_Native(Entity.ID));
                return result;
            }
            set
            {
                IntPtr ptr = value == null ? IntPtr.Zero : value.m_UnmanagedInstance;
                SetMesh_Native(Entity.ID, ptr);
            }
        }
        public unsafe static IntPtr GetMesh_Native(ulong id)
        {
            return InternalCalls.Prism_MeshComponent_GetMesh(id);
        }
        public unsafe static void SetMesh_Native(ulong id, IntPtr unmanagedInstance)
        {
            InternalCalls.Prism_MeshComponent_SetMesh(id, unmanagedInstance);
        }

        public MaterialInstance GetMaterial(int index)
        {
            return Mesh.GetMaterial(index);
        }

        public void SetMaterial(int index, MaterialInstance material)
        {
            if (Mesh != null)
                Mesh.SetMaterial(index, material);
        }

        public int GetMaterialCount()
        {
            return Mesh?.GetMaterialCount() ?? 0;
        }

        public void SetOverrideMaterial(MaterialInstance material)
        {
            if (Mesh != null)
                Mesh.SetOverrideMaterial(material);
        }

        public MaterialInstance GetOverrideMaterial()
        {
            return Mesh.GetOverrideMaterial();
        }

    }

    public class CameraComponent : Component
    {
        // TODO
    }

    public class MaterialComponent : Component
    {
        public MaterialInstance Material
        {
            get
            {
                IntPtr ptr;
                unsafe
                {
                    ptr = InternalCalls.Prism_MaterialComponent_GetMaterial(Entity.ID);
                }
                if (ptr == IntPtr.Zero)
                    throw new NullReferenceException("MaterialInstance Not Exists");
                return new MaterialInstance(ptr);
            }
            set
            {
                unsafe
                {
                    InternalCalls.Prism_MaterialComponent_SetMaterial(Entity.ID, value == null ? IntPtr.Zero : value.m_UnmanagedInstance);
                }
            }
        }
    }

    public class ScriptComponent : Component
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
