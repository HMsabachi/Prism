using System;
using System.Runtime.InteropServices;

namespace Prism
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3 : IEquatable<Vector3>, IFormattable
    {
        public float X;
        public float Y;
        public float Z;

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public Vector3(float scalar)
        {
            X = Y = Z = scalar;
        }

        public Vector3(Vector2 vector)
        {
            X = vector.X;
            Y = vector.Y;
            Z = 0f;
        }

        public Vector3(float scalar, Vector2 yz)
        {
            X = scalar;
            Y = yz.X;
            Z = yz.Y;
        }

        public Vector3(Vector4 vector)
        {
            X = vector.X;
            Y = vector.Y;
            Z = vector.Z;
        }

        public static Vector3 Zero => new(0f, 0f, 0f);
        public static Vector3 One => new(1f, 1f, 1f);
        public static Vector3 Up => new(0f, 1f, 0f);
        public static Vector3 Down => new(0f, -1f, 0f);
        public static Vector3 Left => new(-1f, 0f, 0f);
        public static Vector3 Right => new(1f, 0f, 0f);
        public static Vector3 Forward => new(0f, 0f, -1f);
        public static Vector3 Back => new(0f, 0f, 1f);

        public float Magnitude => MathF.Sqrt(SqrMagnitude);
        public float SqrMagnitude => X * X + Y * Y + Z * Z;

        public Vector3 Normalized
        {
            get
            {
                float mag = Magnitude;
                if (mag > 1E-6f)
                    return new Vector3(X / mag, Y / mag, Z / mag);
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
            }
        }

        public void Clamp(Vector3 min, Vector3 max)
        {
            X = Mathf.Clamp(X, min.X, max.X);
            Y = Mathf.Clamp(Y, min.Y, max.Y);
            Z = Mathf.Clamp(Z, min.Z, max.Z);
        }

        public Vector2 XY
        {
            get => new(X, Y);
            set { X = value.X; Y = value.Y; }
        }

        public Vector2 XZ
        {
            get => new(X, Z);
            set { X = value.X; Z = value.Y; }
        }

        public Vector2 YZ
        {
            get => new(Y, Z);
            set { Y = value.X; Z = value.Y; }
        }

        public static float Distance(Vector3 a, Vector3 b)
            => (a - b).Magnitude;

        public static float Dot(Vector3 a, Vector3 b)
            => a.X * b.X + a.Y * b.Y + a.Z * b.Z;

        public static Vector3 Cross(Vector3 a, Vector3 b)
            => new(
                a.Y * b.Z - a.Z * b.Y,
                a.Z * b.X - a.X * b.Z,
                a.X * b.Y - a.Y * b.X
            );

        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
            => LerpUnclamped(a, b, Math.Clamp(t, 0f, 1f));

        public static Vector3 LerpUnclamped(Vector3 a, Vector3 b, float t)
            => new(a.X + (b.X - a.X) * t, a.Y + (b.Y - a.Y) * t, a.Z + (b.Z - a.Z) * t);

        public static Vector3 MoveTowards(Vector3 current, Vector3 target, float maxDistanceDelta)
        {
            Vector3 delta = target - current;
            float sqrDist = delta.SqrMagnitude;
            if (sqrDist == 0f || (maxDistanceDelta >= 0f && sqrDist <= maxDistanceDelta * maxDistanceDelta))
                return target;
            float dist = MathF.Sqrt(sqrDist);
            return current + delta / dist * maxDistanceDelta;
        }

        public static float Angle(Vector3 from, Vector3 to)
        {
            float dot = Dot(from.Normalized, to.Normalized);
            return MathF.Acos(Math.Clamp(dot, -1f, 1f)) * (180f / MathF.PI);
        }

        public static Vector3 Scale(Vector3 a, Vector3 b)
            => new(a.X * b.X, a.Y * b.Y, a.Z * b.Z);

        public static Vector3 Project(Vector3 vector, Vector3 onNormal)
        {
            float sqrMag = onNormal.SqrMagnitude;
            if (sqrMag < 1E-6f)
                return Zero;
            return onNormal * Dot(vector, onNormal) / sqrMag;
        }

        public static Vector3 Reflect(Vector3 inDirection, Vector3 inNormal)
            => -2f * Dot(inNormal, inDirection) * inNormal + inDirection;

        public static Vector3 ClampMagnitude(Vector3 vector, float maxLength)
        {
            float sqrMag = vector.SqrMagnitude;
            if (sqrMag > maxLength * maxLength)
            {
                float mag = MathF.Sqrt(sqrMag);
                return vector / mag * maxLength;
            }
            return vector;
        }

        // Component-wise math — used by Quaternion euler constructor
        public static Vector3 Cos(Vector3 v)
            => new(MathF.Cos(v.X), MathF.Cos(v.Y), MathF.Cos(v.Z));

        public static Vector3 Sin(Vector3 v)
            => new(MathF.Sin(v.X), MathF.Sin(v.Y), MathF.Sin(v.Z));

        public static Vector3 operator +(Vector3 a, Vector3 b)
            => new(a.X + b.X, a.Y + b.Y, a.Z + b.Z);

        public static Vector3 operator -(Vector3 a, Vector3 b)
            => new(a.X - b.X, a.Y - b.Y, a.Z - b.Z);

        public static Vector3 operator -(Vector3 v)
            => new(-v.X, -v.Y, -v.Z);

        public static Vector3 operator *(Vector3 a, float d)
            => new(a.X * d, a.Y * d, a.Z * d);

        public static Vector3 operator *(Vector3 a, Vector3 b)
            => new(a.X * b.X, a.Y * b.Y, a.Z * b.Z);

        public static Vector3 operator *(float d, Vector3 a)
            => new(a.X * d, a.Y * d, a.Z * d);

        public static Vector3 operator /(Vector3 a, float d)
            => new(a.X / d, a.Y / d, a.Z / d);

        public static bool operator ==(Vector3 a, Vector3 b)
            => a.X == b.X && a.Y == b.Y && a.Z == b.Z;

        public static bool operator !=(Vector3 a, Vector3 b)
            => !(a == b);

        public override bool Equals(object? obj)
            => obj is Vector3 other && Equals(other);

        public override int GetHashCode()
            => HashCode.Combine(X, Y, Z);

        public bool Equals(Vector3 other)
            => X == other.X && Y == other.Y && Z == other.Z;

        public override string ToString()
            => $"({X}, {Y}, {Z})";

        public string ToString(string? format, IFormatProvider? formatProvider)
            => $"({X.ToString(format, formatProvider)}, {Y.ToString(format, formatProvider)}, {Z.ToString(format, formatProvider)})";
    }
}
