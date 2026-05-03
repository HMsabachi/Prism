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

    public class TransformComponent : Component
    {
        public Matrix4 Transform
        {
            get
            {
                Matrix4 matrix;
                unsafe
                {
                    InternalCalls.Prism_Entity_GetTransform(Entity.ID, &matrix);
                }
                return matrix;
            }
            set
            {
                unsafe
                {
                    InternalCalls.Prism_Entity_SetTransform(Entity.ID, &value);
                }
            }
        }
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
                    return null;
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
}
