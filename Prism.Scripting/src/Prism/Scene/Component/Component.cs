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
        public Material Material
        {
            get
            {
                IntPtr ptr = IntPtr.Zero;
                unsafe { InternalCalls.Prism_MeshRendererComponent_GetMaterial(Entity.ID, &ptr, 0); }
                return new Material(ptr);
            }
            set
            {
                IntPtr ptr = value.m_UnmanagedInstance;
                unsafe { InternalCalls.Prism_MeshRendererComponent_SetMaterial(Entity.ID, ptr, 0); }
            }
        }
        public Material[] Materials
        {
            get
            {
                unsafe
                {
                    UInt64 count = InternalCalls.Prism_MeshRendererComponent_GetMaterialCount(Entity.ID);
                    var materials = new Material[count];
                    if (count == 0) return materials;
                    var handles = new IntPtr[count];
                    fixed (IntPtr* p = handles)
                        InternalCalls.Prism_MeshRendererComponent_GetMaterials(Entity.ID, p);
                    for (UInt64 i = 0; i < count; i++)
                        materials[i] = new Material(handles[i]);
                    return materials;
                }
            }
            set
            {
                unsafe
                {
                    int count = value.Length;
                    var handles = new IntPtr[count];
                    for (int i = 0; i < count; i++)
                        handles[i] = value[i] != null ? value[i].m_UnmanagedInstance : IntPtr.Zero;
                    fixed (IntPtr* p = handles)
                        InternalCalls.Prism_MeshRendererComponent_SetMaterials(Entity.ID, p, (uint)count);
                }
            }
        }

        public Material GetMaterial(int index)
        {
            unsafe
            {
                IntPtr ptr = IntPtr.Zero;
                InternalCalls.Prism_MeshRendererComponent_GetMaterial(Entity.ID, &ptr, (uint)index);
                return new Material(ptr);
            }
        }

        public void SetMaterial(int index, Material material)
        {
            IntPtr ptr = material != null ? material.m_UnmanagedInstance : IntPtr.Zero;
            unsafe { InternalCalls.Prism_MeshRendererComponent_SetMaterial(Entity.ID, ptr, (uint)index); }
        }

        public int GetMaterialCount()
        {
            unsafe { return (int)InternalCalls.Prism_MeshRendererComponent_GetMaterialCount(Entity.ID); }
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
        public enum Type
        {
            Static,
            Dynamic
        }

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

        public Type BodyType
        {
            get { unsafe { return InternalCalls.Prism_RigidBodyComponent_GetBodyType(Entity.ID); } }
        }

        public Vector3 AngularVelocity
        {
            get
            {
                Vector3 result;
                unsafe { InternalCalls.Prism_RigidBodyComponent_GetAngularVelocity(Entity.ID, &result); }
                return result;
            }
            set
            {
                unsafe { InternalCalls.Prism_RigidBodyComponent_SetAngularVelocity(Entity.ID, &value); }
            }
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
