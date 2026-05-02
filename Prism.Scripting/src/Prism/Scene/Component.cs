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
        public required Entity Entity { get; set; }
        
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
        private Matrix4 _value;
        public Matrix4 Transform
        {
            get
            {
                unsafe
                {
                    fixed (Matrix4* ptr = &_value) InternalCalls.Prism_Entity_GetTransform(Entity.ID, ptr);
                }
                return _value;
            }
            set
            {
                _value = value;
                unsafe
                {
                    fixed (Matrix4* ptr = &_value) InternalCalls.Prism_Entity_SetTransform(Entity.ID, ptr);
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

    public class ScriptComponent : Component
    {
        // TODO
    }

    public class SpriteRendererComponent : Component
    {
        // TODO
    }
}