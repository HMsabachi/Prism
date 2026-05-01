using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
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
        private Matrix4 _value;
        public Matrix4 Transform
        {
            get
            {
                GetTransform_Native(Entity.ID, out _value);
                return _value;
            }
            set
            {
                _value = value;
                SetTransform_Native(Entity.ID, ref _value);
            }
        }

        public unsafe static void GetTransform_Native(ulong id, out Matrix4 result)
        {
            fixed (Matrix4* ptr = &result) InternalCalls.Prism_Entity_GetTransform(id, ptr);
        }

        public unsafe static void SetTransform_Native(ulong id, ref Matrix4 result)
        {
            fixed (Matrix4* ptr = &result) InternalCalls.Prism_Entity_SetTransform(id, ptr);
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