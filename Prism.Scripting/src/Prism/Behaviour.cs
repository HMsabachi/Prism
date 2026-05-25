using System;

namespace Prism
{
    public abstract class Behaviour : Component
    {
        public bool Enabled { get; set; } = true;

        public virtual void Awake() { }
        public virtual void OnEnable() { }
        public virtual void OnDisable() { }
        public virtual void OnCreate() { }
        public virtual void OnUpdate() { }
        public virtual void LateUpdate() { }
        public virtual void OnFixedUpdate() { }
        public virtual void OnDestroy() { }
        public virtual void OnCollisionBegin(float data) { }
        public virtual void OnCollisionEnd(float data) { }
    }
}
