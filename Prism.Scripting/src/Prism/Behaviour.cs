using System;

namespace Prism
{
    public abstract class Behaviour : Component
    {
        public ulong ID;

        public bool Enabled
        {
            get
            {
                if (ID == 0) return true;
                unsafe { return InternalCalls.Prism_Behaviour_GetEnabled(ID); }
            }
            set
            {
                if (ID == 0) return;
                unsafe { InternalCalls.Prism_Behaviour_SetEnabled(ID, value); }
            }
        }

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
