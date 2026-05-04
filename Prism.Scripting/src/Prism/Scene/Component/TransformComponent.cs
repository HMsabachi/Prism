
using System;

namespace Prism
{
    public class TransformComponent : Component
    {
        public Matrix4 Transform
        {
            get
            {
                Matrix4 matrix;
                unsafe
                {
                    InternalCalls.Prism_Entity_GetTransform(Entity.ID, &matrix);
                }
                return matrix;
            }
            set
            {
                unsafe
                {
                    InternalCalls.Prism_Entity_SetTransform(Entity.ID, &value);
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

        // Position axis accessors — temporary workaround.
        // TODO: 后续改为 ref return / 直接内存修改模式，实现 Unity 式 transform.position.X = value 语法
        public float PositionX { get => Position.X; set => Position = new Vector3(value, Position.Y, Position.Z); }
        public float PositionY { get => Position.Y; set => Position = new Vector3(Position.X, value, Position.Z); }
        public float PositionZ { get => Position.Z; set => Position = new Vector3(Position.X, Position.Y, value); }

        public Vector3 Rotation
        {
            get
            {
                Vector3 rotation;
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_GetRotation(Entity.ID, &rotation);
                }
                return new Vector3(
                    rotation.X * (180.0f / MathF.PI),
                    rotation.Y * (180.0f / MathF.PI),
                    rotation.Z * (180.0f / MathF.PI)
                );
            }
            set
            {
                unsafe
                {
                    InternalCalls.Prism_TransformComponent_SetRotation(Entity.ID, value);
                }
            }
        }

        // Rotation axis accessors — temporary workaround.
        // TODO: 后续改为 ref return / 直接内存修改模式，实现 Unity 式 transform.rotation.X = value 语法
        public float RotationX { get => Rotation.X; set => Rotation = new Vector3(value, Rotation.Y, Rotation.Z); }
        public float RotationY { get => Rotation.Y; set => Rotation = new Vector3(Rotation.X, value, Rotation.Z); }
        public float RotationZ { get => Rotation.Z; set => Rotation = new Vector3(Rotation.X, Rotation.Y, value); }

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

        // Scale axis accessors — temporary workaround.
        // TODO: 后续改为 ref return / 直接内存修改模式，实现 Unity 式 transform.scale.X = value 语法
        public float ScaleX { get => Scale.X; set => Scale = new Vector3(value, Scale.Y, Scale.Z); }
        public float ScaleY { get => Scale.Y; set => Scale = new Vector3(Scale.X, value, Scale.Z); }
        public float ScaleZ { get => Scale.Z; set => Scale = new Vector3(Scale.X, Scale.Y, value); }

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
    }
}
