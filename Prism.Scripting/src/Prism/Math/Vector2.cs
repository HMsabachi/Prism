using System;
using System.Runtime.InteropServices;

namespace Prism
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2 : IEquatable<Vector2>, IFormattable
    {
        public float X;
        public float Y;

        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }

        public Vector2(float scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public Vector2(Vector3 vector)
        {
            X = vector.X;
            Y = vector.Y;
        }

        public static Vector2 Zero => new(0f, 0f);
        public static Vector2 One => new(1f, 1f);
        public static Vector2 Up => new(0f, 1f);
        public static Vector2 Down => new(0f, -1f);
        public static Vector2 Left => new(-1f, 0f);
        public static Vector2 Right => new(1f, 0f);

        public float Magnitude => MathF.Sqrt(SqrMagnitude);
        public float SqrMagnitude => X * X + Y * Y;

        public Vector2 Normalized
        {
            get
            {
                float mag = Magnitude;
                if (mag > 1E-6f)
                    return new Vector2(X / mag, Y / mag);
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
            }
        }

        public static float Distance(Vector2 a, Vector2 b)
            => (a - b).Magnitude;

        public static float Dot(Vector2 a, Vector2 b)
            => a.X * b.X + a.Y * b.Y;

        public static Vector2 Lerp(Vector2 a, Vector2 b, float t)
            => LerpUnclamped(a, b, Math.Clamp(t, 0f, 1f));

        public static Vector2 LerpUnclamped(Vector2 a, Vector2 b, float t)
            => new(a.X + (b.X - a.X) * t, a.Y + (b.Y - a.Y) * t);

        public static Vector2 MoveTowards(Vector2 current, Vector2 target, float maxDistanceDelta)
        {
            Vector2 delta = target - current;
            float sqrDist = delta.SqrMagnitude;
            if (sqrDist == 0f || (maxDistanceDelta >= 0f && sqrDist <= maxDistanceDelta * maxDistanceDelta))
                return target;
            float dist = MathF.Sqrt(sqrDist);
            return current + delta / dist * maxDistanceDelta;
        }

        public static Vector2 Scale(Vector2 a, Vector2 b)
            => new(a.X * b.X, a.Y * b.Y);

        public static Vector2 Reflect(Vector2 inDirection, Vector2 inNormal)
            => -2f * Dot(inNormal, inDirection) * inNormal + inDirection;

        public static Vector2 Perpendicular(Vector2 inDirection)
            => new(-inDirection.Y, inDirection.X);

        public static Vector2 Clamp(Vector2 value, Vector2 min, Vector2 max)
        {
            return new Vector2(
                Math.Clamp(value.X, min.X, max.X),
                Math.Clamp(value.Y, min.Y, max.Y)
            );
        }

        public void Clamp(Vector2 min, Vector2 max)
        {
            X = Math.Clamp(X, min.X, max.X);
            Y = Math.Clamp(Y, min.Y, max.Y);
        }

        public static bool EpsilonEquals(Vector2 a, Vector2 b)
            => MathF.Abs(a.X - b.X) < 1e-6f && MathF.Abs(a.Y - b.Y) < 1e-6f;

        public static Vector2 operator +(Vector2 a, Vector2 b)
            => new(a.X + b.X, a.Y + b.Y);

        public static Vector2 operator -(Vector2 a, Vector2 b)
            => new(a.X - b.X, a.Y - b.Y);

        public static Vector2 operator -(Vector2 v)
            => new(-v.X, -v.Y);

        public static Vector2 operator *(Vector2 a, float d)
            => new(a.X * d, a.Y * d);

        public static Vector2 operator *(float d, Vector2 a)
            => new(a.X * d, a.Y * d);

        public static Vector2 operator /(Vector2 a, float d)
            => new(a.X / d, a.Y / d);

        public static bool operator ==(Vector2 a, Vector2 b)
            => a.X == b.X && a.Y == b.Y;

        public static bool operator !=(Vector2 a, Vector2 b)
            => !(a == b);

        public override bool Equals(object? obj)
            => obj is Vector2 other && Equals(other);

        public override int GetHashCode()
            => HashCode.Combine(X, Y);

        public bool Equals(Vector2 other)
            => X == other.X && Y == other.Y;

        public override string ToString()
            => $"({X}, {Y})";

        public string ToString(string? format, IFormatProvider? formatProvider)
            => $"({X.ToString(format, formatProvider)}, {Y.ToString(format, formatProvider)})";
    }
}
