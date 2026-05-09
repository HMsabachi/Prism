using System;
using System.Runtime.InteropServices;

namespace Prism
{
    [StructLayout(LayoutKind.Explicit)]
    public struct Vector4 : IEquatable<Vector4>, IFormattable
    {
        [FieldOffset(0)] public float X;
        [FieldOffset(4)] public float Y;
        [FieldOffset(8)] public float Z;
        [FieldOffset(12)] public float W;

        public Vector4(float scalar)
        {
            X = Y = Z = W = scalar;
        }

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public static Vector4 Zero => new(0f, 0f, 0f, 0f);
        public static Vector4 One => new(1f, 1f, 1f, 1f);

        public float Magnitude => MathF.Sqrt(SqrMagnitude);
        public float SqrMagnitude => X * X + Y * Y + Z * Z + W * W;

        public Vector4 Normalized
        {
            get
            {
                float mag = Magnitude;
                if (mag > 1E-6f)
                    return new Vector4(X / mag, Y / mag, Z / mag, W / mag);
                return Zero;
            }
        }

        public void Normalize()
        {
            float mag = Magnitude;
            if (mag > 1E-6f)
            {
                X /= mag;
                Y /= mag;
                Z /= mag;
                W /= mag;
            }
        }

        public void Clamp(Vector4 min, Vector4 max)
        {
            X = Mathf.Clamp(X, min.X, max.X);
            Y = Mathf.Clamp(Y, min.Y, max.Y);
            Z = Mathf.Clamp(Z, min.Z, max.Z);
            W = Mathf.Clamp(W, min.W, max.W);
        }

        public static float Distance(Vector4 a, Vector4 b)
            => (a - b).Magnitude;

        public static float Dot(Vector4 a, Vector4 b)
            => a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;

        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
            => LerpUnclamped(a, b, Math.Clamp(t, 0f, 1f));

        public static Vector4 LerpUnclamped(Vector4 a, Vector4 b, float t)
        {
            return new Vector4(
                a.X + (b.X - a.X) * t,
                a.Y + (b.Y - a.Y) * t,
                a.Z + (b.Z - a.Z) * t,
                a.W + (b.W - a.W) * t
            );
        }

        public static Vector4 MoveTowards(Vector4 current, Vector4 target, float maxDistanceDelta)
        {
            Vector4 delta = target - current;
            float sqrDist = delta.SqrMagnitude;
            if (sqrDist == 0f || (maxDistanceDelta >= 0f && sqrDist <= maxDistanceDelta * maxDistanceDelta))
                return target;
            float dist = MathF.Sqrt(sqrDist);
            return current + delta / dist * maxDistanceDelta;
        }

        public static Vector4 ClampMagnitude(Vector4 vector, float maxLength)
        {
            float sqrMag = vector.SqrMagnitude;
            if (sqrMag > maxLength * maxLength)
            {
                float mag = MathF.Sqrt(sqrMag);
                return vector / mag * maxLength;
            }
            return vector;
        }

        public static Vector4 operator +(Vector4 left, Vector4 right)
            => new(left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W);

        public static Vector4 operator -(Vector4 left, Vector4 right)
            => new(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);

        public static Vector4 operator *(Vector4 left, Vector4 right)
            => new(left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W);

        public static Vector4 operator *(Vector4 left, float scalar)
            => new(left.X * scalar, left.Y * scalar, left.Z * scalar, left.W * scalar);

        public static Vector4 operator *(float scalar, Vector4 right)
            => new(scalar * right.X, scalar * right.Y, scalar * right.Z, scalar * right.W);

        public static Vector4 operator /(Vector4 left, Vector4 right)
            => new(left.X / right.X, left.Y / right.Y, left.Z / right.Z, left.W / right.W);

        public static Vector4 operator /(Vector4 left, float scalar)
            => new(left.X / scalar, left.Y / scalar, left.Z / scalar, left.W / scalar);

        public static bool operator ==(Vector4 a, Vector4 b)
            => a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;

        public static bool operator !=(Vector4 a, Vector4 b)
            => !(a == b);

        public override bool Equals(object? obj)
            => obj is Vector4 other && Equals(other);

        public override int GetHashCode()
            => HashCode.Combine(X, Y, Z, W);

        public bool Equals(Vector4 other)
            => X == other.X && Y == other.Y && Z == other.Z && W == other.W;

        public override string ToString()
            => $"({X}, {Y}, {Z}, {W})";

        public string ToString(string? format, IFormatProvider? formatProvider)
            => $"({X.ToString(format, formatProvider)}, {Y.ToString(format, formatProvider)}, {Z.ToString(format, formatProvider)}, {W.ToString(format, formatProvider)})";
    }
}
