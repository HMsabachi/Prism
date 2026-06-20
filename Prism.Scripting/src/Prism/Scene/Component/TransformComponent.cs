
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
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_GetTransform(Entity.ID, &t);
                }
                return t;
            }
            set
            {
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_SetTransform(Entity.ID, &value);
                }
            }
        }

        public Vector3 Position
        {
            get
            {
                Vector3 position;
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_GetPosition(Entity.ID, &position);
                }
                return position;
            }
            set
            {
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_SetPosition(Entity.ID, value);
                }
            }
        }

        public Vector3 Rotation
        {
            get
            {
                Vector3 rotation;
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_GetRotation(Entity.ID, &rotation);
                }
                return rotation;
            }
            set
            {
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_SetRotation(Entity.ID, value);
                }
            }
        }

        public Vector3 Scale
        {
            get
            {
                Vector3 scale;
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_GetScale(Entity.ID, &scale);
                }
                return scale;
            }
            set
            {
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_SetScale(Entity.ID, value);
                }
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
            get => new(Position, Rotation, Scale);
            set { Position = value.Position; Rotation = value.Rotation; Scale = value.Scale; }
        }

        // Derived direction vectors from current rotation
        public Vector3 Forward => new Quaternion(Rotation * MathF.PI / 180f) * Vector3.Forward;
        public Vector3 Right => new Quaternion(Rotation * MathF.PI / 180f) * Vector3.Right;
        public Vector3 Up => new Quaternion(Rotation * MathF.PI / 180f) * Vector3.Up;
    }
}
