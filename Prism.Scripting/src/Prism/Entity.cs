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
        private ulong m_ID;
        public ulong ID { get { return m_ID; } internal set { m_ID = value; Log.Trace("Created Entity {0}", ID); } }

        public Entity() => m_ID = 0;
        internal Entity(ulong id) => ID = id;
        ~Entity() => Log.Trace("Destroyed Entity {0}", ID);

        public TransformComponent Transform => GetComponent<TransformComponent>();

        public bool HasComponent<T>() where T : Component, new()
        {
            if (typeof(T).IsSubclassOf(typeof(Behaviour)))
            {
                unsafe { return InternalCalls.Prism_Entity_GetBehaviour(ID, typeof(T)) != IntPtr.Zero; }
            }
            unsafe { return InternalCalls.Prism_Entity_HasComponent(ID, typeof(T)); }
        }
        public T CreateComponent<T>() where T : Component, new()
        {
            if (typeof(T).IsSubclassOf(typeof(Behaviour)))
            {
                unsafe
                {
                    NativeString className = typeof(T).FullName;
                    IntPtr handle = InternalCalls.Prism_Entity_AddBehaviour(ID, className);
                    if (handle == IntPtr.Zero) throw new MissingReferenceException();
                    object? obj = GCHandle.FromIntPtr(handle).Target;
                    if (obj == null) throw new NullReferenceException();
                    T behaviour = (T)obj;
                    behaviour.Entity = this;
                    return behaviour;
                }
            }
            else
            {
                unsafe { InternalCalls.Prism_Entity_CreateComponent(ID, typeof(T)); }
                T component = new T();
                component.Entity = this;
                return component;
            }
        }
        public T GetComponent<T>() where T : Component, new()
        {
            if (typeof(T).IsSubclassOf(typeof(Behaviour)))
            {
                unsafe
                {
                    IntPtr handle = InternalCalls.Prism_Entity_GetBehaviour(ID, typeof(T));
                    if (handle == IntPtr.Zero) throw new NullReferenceException();
                    object? behaviour = GCHandle.FromIntPtr(handle).Target;
                    if (behaviour == null) throw new NullReferenceException();
                    return (T)behaviour;
                }
            }
            if (HasComponent<T>())
            {
                T component = new T();
                component.Entity = this;
                return component;
            }
            throw new NullReferenceException();
        }

        public Entity FindEntityByTag(string tag)
        {
            unsafe
            {
                NativeString nativeTag = tag;
                ulong entityID = InternalCalls.Prism_Entity_FindEntityByTag(nativeTag);
                if (entityID == 0)
                    throw new EntityNotFoundException();
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
                    throw new EntityNotFoundException();
                return new Entity(entityID);
            }
        }

        public static Entity FindEntityByID(ulong entityID)
        {
            // TODO: Verify the entity id
            return new Entity(entityID);
        }
    }
}
