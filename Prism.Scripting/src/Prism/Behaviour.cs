using System;

namespace Prism
{
    public abstract class Behaviour : Component
    {
        public bool Enabled { get; set; } = true;

        public TransformComponent Transform => Entity.Transform;

        public T GetComponent<T>() where T : Component, new()
        {
            return Entity.GetComponent<T>();
        }
        public bool HasComponent<T>() where T : Component, new()
        {
            return Entity.HasComponent<T>();
        }
        public T CreateComponent<T>() where T : Component, new()
        {
            return Entity.CreateComponent<T>();
        }
    }
}
