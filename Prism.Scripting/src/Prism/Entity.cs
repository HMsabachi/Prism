using System;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Prism
{
    public class Entity
    {
        public uint SceneID { get; internal set; }
        public uint EntityID { get; internal set; }

        ~Entity()
        {
            Console.WriteLine("Destroyed Entity {0}:{1}", SceneID, EntityID);
        }

        public Matrix4 GetTransform()
        {
            Matrix4 mat4Instance;
            TransformComponent.GetTransform_Native(SceneID, EntityID, out mat4Instance);
            return mat4Instance;
        }

        public void SetTransform(Matrix4 transform)
        {
            TransformComponent.SetTransform_Native(SceneID, EntityID, ref transform);
        }
        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(SceneID, EntityID, typeof(T));
        }
        public T GetComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
            {
                T component = new T();
                component.Entity = this;
                return component;
            }
            return null;
        }

        private unsafe static bool HasComponent_Native(uint sceneID, uint entityID, Type type)
        {
            // TODO: Implement this in native code
            return true;
        }
    }
}
