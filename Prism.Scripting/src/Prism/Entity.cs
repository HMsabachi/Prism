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

        public Entity() 
        { 
            ID = 0; 
            Log.Trace("Created Entity {0}", ID); 
        }

        internal Entity(ulong id)
        {
            ID = id;
            Log.Trace("Created Entity {0}", ID);
        }

        ~Entity()
        {
            Log.Trace("Destroyed Entity {0}", ID);
        }

        public TransformComponent Transform => GetComponent<TransformComponent>();

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
