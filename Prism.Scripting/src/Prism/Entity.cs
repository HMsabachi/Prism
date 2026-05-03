using System;
using System.Collections.Generic;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Rolky.Managed.Interop;
namespace Prism
{
    public class Entity
    {
        public ulong ID { get; internal set; }

        private List<Action<float>> m_Collision2DBeginCallbacks = new List<Action<float>>();
        private List<Action<float>> m_Collision2DEndCallbacks = new List<Action<float>>();

        protected Entity() { ID = 0; }

        internal Entity(ulong id)
        {
            ID = id;
        }

        ~Entity()
        {
            Console.WriteLine("Destroyed Entity {0}", ID);
        }

        public Matrix4 GetTransform()
        {
            var transform = GetComponent<TransformComponent>();
            return transform.Transform;
        }

        public void SetTransform(Matrix4 value)
        {
            var transform = GetComponent<TransformComponent>();
            transform.Transform = value;
        }
        public bool HasComponent<T>() where T : Component, new()
        {
            unsafe { return InternalCalls.Prism_Entity_HasComponent(ID, typeof(T)); }
        }
        public T CreateComponent<T>() where T : Component, new()
        {
            unsafe { InternalCalls.Prism_Entity_CreateComponent(ID, typeof(T));}
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

        public Entity FindEntityByTag(string tag)
        {
            unsafe
            {
                NativeString nativeTag = tag;
                ulong entityID = InternalCalls.Prism_Entity_FindEntityByTag(nativeTag);
                if (entityID == 0)
                    return null;
                return new Entity(entityID);
            }
        }

        public void AddCollision2DBeginCallback(Action<float> callback)
        {
            m_Collision2DBeginCallbacks.Add(callback);
        }

        public void AddCollision2DEndCallback(Action<float> callback)
        {
            m_Collision2DEndCallbacks.Add(callback);
        }

        private void OnCollision2DBegin(float data)
        {
            foreach (var callback in m_Collision2DBeginCallbacks)
                callback.Invoke(data);
        }

        private void OnCollision2DEnd(float data)
        {
            foreach (var callback in m_Collision2DEndCallbacks)
                callback.Invoke(data);
        }

        // Entity lookup
        public static Entity FindEntityByTagStatic(string tag)
        {
            unsafe
            {
                NativeString nativeTag = tag;
                ulong entityID = InternalCalls.Prism_Entity_FindEntityByTag(nativeTag);
                if (entityID == 0)
                    return null;
                return new Entity(entityID);
            }
        }
    }
}
