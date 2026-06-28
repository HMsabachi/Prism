
using System;

namespace Prism
{
    public class TransformComponent : Component
    {
        public Transform Transform
        {
            get
            {
                Transform t;
                unsafe { InternalCalls.Prism_TransformComponent_GetTransform(Entity.ID, &t); }
                return t;
            }
            set
            {
                unsafe { InternalCalls.Prism_TransformComponent_SetTransform(Entity.ID, &value); }
            }
        }

        public Vector3 Position
        {
            get
            {
                Vector3 position;
                unsafe { InternalCalls.Prism_TransformComponent_GetPosition(Entity.ID, &position); }
                return position;
            }
            set
            {
                unsafe { InternalCalls.Prism_TransformComponent_SetPosition(Entity.ID, &value); }
            }
        }

        public Vector3 Rotation
        {
            get
            {
                Vector3 rotation;
                unsafe { InternalCalls.Prism_TransformComponent_GetRotation(Entity.ID, &rotation); }
                return rotation;
            }
            set
            {
                unsafe { InternalCalls.Prism_TransformComponent_SetRotation(Entity.ID, &value); }
            }
        }

        public Vector3 Scale
        {
            get
            {
                Vector3 scale;
                unsafe { InternalCalls.Prism_TransformComponent_GetScale(Entity.ID, &scale); }
                return scale;
            }
            set
            {
                unsafe { InternalCalls.Prism_TransformComponent_SetScale(Entity.ID, &value); }
            }
        }

        public Vector3 LocalPosition
        {
            get
            {
                Vector3 position;
                unsafe { InternalCalls.Prism_TransformComponent_GetLocalPosition(Entity.ID, &position); }
                return position;
            }
            set
            {
                unsafe { InternalCalls.Prism_TransformComponent_SetLocalPosition(Entity.ID, &value); }
            }
        }

        public Vector3 LocalRotation
        {
            get
            {
                Vector3 rotation;
                unsafe { InternalCalls.Prism_TransformComponent_GetLocalRotation(Entity.ID, &rotation); }
                return rotation;
            }
            set
            {
                unsafe { InternalCalls.Prism_TransformComponent_SetLocalRotation(Entity.ID, &value); }
            }
        }

        public Vector3 LocalScale
        {
            get
            {
                Vector3 scale;
                unsafe { InternalCalls.Prism_TransformComponent_GetLocalScale(Entity.ID, &scale); }
                return scale;
            }
            set
            {
                unsafe { InternalCalls.Prism_TransformComponent_SetLocalScale(Entity.ID, &value); }
            }
        }

        public void SetPosition(float x, float y, float z)
        {
            Position = new Vector3(x, y, z);
        }

        public void SetRotation(float x, float y, float z)
        {
            Rotation = new Vector3(x, y, z);
        }

        public void SetScale(float x, float y, float z)
        {
            Scale = new Vector3(x, y, z);
        }

        public Transform LocalTransform
        {
            get
            {
                return new Transform(LocalPosition, LocalRotation, LocalScale,
                                     Vector3.Zero, Vector3.Zero, Vector3.Zero);
            }
            set { LocalPosition = value.Position; LocalRotation = value.Rotation; LocalScale = value.Scale; }
        }

        public Vector3 Forward
        {
            get
            {
                Vector3 forward;
                unsafe { InternalCalls.Prism_TransformComponent_GetForward(Entity.ID, &forward); }
                return forward;
            }
        }
        public Vector3 Right
        {
            get
            {
                Vector3 right;
                unsafe { InternalCalls.Prism_TransformComponent_GetRight(Entity.ID, &right); }
                return right;
            }
        }
        public Vector3 Up
        {
            get
            {
                Vector3 up;
                unsafe { InternalCalls.Prism_TransformComponent_GetUp(Entity.ID, &up); }
                return up;
            }
        }
    }
}
