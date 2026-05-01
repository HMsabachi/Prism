using System;
using System.Numerics;
using System.Runtime.CompilerServices;

using Rolky.Managed.Interop;
namespace Prism
{
    public class Entity
    {
        public ulong ID { get; internal set; }

        ~Entity()
        {
            Console.WriteLine("Destroyed Entity {0}", ID);
        }

        public Matrix4 GetTransform()
        {
            Matrix4 mat4Instance;
            TransformComponent.GetTransform_Native(ID, out mat4Instance);
            return mat4Instance;
        }

        public void SetTransform(Matrix4 transform)
        {
            TransformComponent.SetTransform_Native(ID, ref transform);
        }
        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(ID, typeof(T));
        }
        public T CreateComponent<T>() where T : Component, new()
        {
            CreateComponent_Native(ID, typeof(T));
            T component = new T();
            component.Entity = this;
            return component;
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
        private unsafe static void CreateComponent_Native(ulong id, Type type)
        {
            InternalCalls.Prism_Entity_CreateComponent(id, type);
        }
        private unsafe static bool HasComponent_Native(ulong id, Type type)
        {
            return InternalCalls.Prism_Entity_HasComponent(id, type);
        }
    }
}
