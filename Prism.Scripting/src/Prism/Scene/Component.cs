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
                return GetTag_Native(Entity.SceneID, Entity.EntityID);
            }
            set
            {
                SetTag_Native(value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetTag_Native(uint sceneID, uint entityID);

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
                GetTransform_Native(Entity.SceneID, Entity.EntityID, out _value);
                return _value;
            }
            set
            {
                _value = value;
                SetTransform_Native(Entity.SceneID, Entity.EntityID, ref _value);
            }
        }

        public unsafe static void GetTransform_Native(uint sceneID, uint entityID, out Matrix4 result)
        {
            fixed (Matrix4* ptr = &result) InternalCalls.Prism_Entity_GetTransform(sceneID, entityID, ptr);
        }

        public unsafe static void SetTransform_Native(uint sceneID, uint entityID, ref Matrix4 result)
        {
            fixed (Matrix4* ptr = &result) InternalCalls.Prism_Entity_SetTransform(sceneID, entityID, ptr);
        }

    }

    public class MeshComponent : Component
    {
       // TODO

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