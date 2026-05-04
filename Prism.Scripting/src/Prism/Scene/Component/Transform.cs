using System;

namespace Prism
{
    public struct Transform : IEquatable<Transform>
    {
        public Vector3 Position;
        public Vector3 Rotation; // euler angles in degrees
        public Vector3 Scale;

        public Vector3 Up => new Quaternion(Rotation * MathF.PI / 180f) * Vector3.Up;
        public Vector3 Right => new Quaternion(Rotation * MathF.PI / 180f) * Vector3.Right;
        public Vector3 Forward => new Quaternion(Rotation * MathF.PI / 180f) * Vector3.Forward;

        public Transform(Vector3 position, Vector3 rotation, Vector3 scale)
        {
            Position = position;
            Rotation = rotation;
            Scale = scale;
        }

        // Compose two transforms: result = a * b  (apply b, then a)
        public static Transform operator *(Transform a, Transform b)
        {
            Quaternion aq = new(a.Rotation * MathF.PI / 180f);
            Quaternion bq = new(b.Rotation * MathF.PI / 180f);
            Quaternion cq = aq * bq;

            Transform result;
            result.Position = a.Position + aq * (a.Scale * b.Position);
            result.Rotation = cq.EulerAngles() * (180f / MathF.PI);
            result.Scale = new Vector3(
                a.Scale.X * b.Scale.X,
                a.Scale.Y * b.Scale.Y,
                a.Scale.Z * b.Scale.Z
            );
            return result;
        }

        public override bool Equals(object? obj) => obj is Transform other && Equals(other);
        public bool Equals(Transform right)
            => Position == right.Position && Rotation == right.Rotation && Scale == right.Scale;
        public override int GetHashCode() => HashCode.Combine(Position, Rotation, Scale);
        public static bool operator ==(Transform left, Transform right) => left.Equals(right);
        public static bool operator !=(Transform left, Transform right) => !(left == right);
    }
}
