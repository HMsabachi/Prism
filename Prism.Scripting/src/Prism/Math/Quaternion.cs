using System;
using System.Runtime.InteropServices;

namespace Prism
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion : IEquatable<Quaternion>
    {
        // X, Y, Z, W order to match glm::quat storage
        public float X;
        public float Y;
        public float Z;
        public float W;

        // Note: constructor order is (x, y, z, w), different from glm's (w, x, y, z)
        public Quaternion(float x, float y, float z, float w)
        {
            X = x; Y = y; Z = z; W = w;
        }

        public Quaternion(Vector3 xyz, float w)
        {
            X = xyz.X; Y = xyz.Y; Z = xyz.Z; W = w;
        }

        // euler angles in radians, Z-X-Y (Yaw-Pitch-Roll) order
        public Quaternion(Vector3 euler)
        {
            Vector3 c = Vector3.Cos(euler * 0.5f);
            Vector3 s = Vector3.Sin(euler * 0.5f);

            X = s.X * c.Y * c.Z - c.X * s.Y * s.Z;
            Y = c.X * s.Y * c.Z + s.X * c.Y * s.Z;
            Z = c.X * c.Y * s.Z - s.X * s.Y * c.Z;
            W = c.X * c.Y * c.Z + s.X * s.Y * s.Z;
        }

        public static Quaternion Identity => new(0f, 0f, 0f, 1f);

        public Quaternion Conjugate => new(-X, -Y, -Z, W);

        public float Length => MathF.Sqrt(LengthSquared);

        public float LengthSquared => X * X + Y * Y + Z * Z + W * W;

        public Vector3 XYZ
        {
            get => new(X, Y, Z);
            set { X = value.X; Y = value.Y; Z = value.Z; }
        }

        public void Normalize()
        {
            float scale = 1f / Length;
            X *= scale; Y *= scale; Z *= scale; W *= scale;
        }

        public static Vector3 operator *(Quaternion q, Vector3 v)
        {
            Vector3 qv = new(q.X, q.Y, q.Z);
            Vector3 uv = Vector3.Cross(qv, v);
            Vector3 uuv = Vector3.Cross(qv, uv);
            return v + ((uv * q.W) + uuv) * 2f;
        }

        public static Quaternion operator *(Quaternion a, Quaternion b)
        {
            return new Quaternion(
                a.W * b.X + a.X * b.W + a.Y * b.Z - a.Z * b.Y,
                a.W * b.Y + a.Y * b.W + a.Z * b.X - a.X * b.Z,
                a.W * b.Z + a.Z * b.W + a.X * b.Y - a.Y * b.X,
                a.W * b.W - a.X * b.X - a.Y * b.Y - a.Z * b.Z
            );
        }

        // Extract euler angles in radians (Z-X-Y order), returns (pitch, yaw, roll)
        public Vector3 EulerAngles()
        {
            float roll = MathF.Atan2(2f * (X * Y + W * Z), W * W + X * X - Y * Y - Z * Z);
            float y = 2f * (Y * Z + W * X);
            float x = W * W - X * X - Y * Y + Z * Z;

            float pitch;
            if (Vector2.EpsilonEquals(new Vector2(x, y), Vector2.Zero))
                pitch = 2f * MathF.Atan2(X, W);
            else
                pitch = MathF.Atan2(y, x);

            float yaw = MathF.Asin(Math.Clamp(-2f * (X * Z - W * Y), -1f, 1f));

            return new Vector3(pitch, yaw, roll);
        }

        public static Quaternion AngleAxis(float angleDeg, Vector3 axis)
        {
            axis.Normalize();
            float rad = angleDeg * MathF.PI / 360f;
            axis *= MathF.Sin(rad);
            return new Quaternion(axis.X, axis.Y, axis.Z, MathF.Cos(rad));
        }

        public static Quaternion FromToRotation(Vector3 from, Vector3 to)
        {
            Vector3 axis = Vector3.Cross(from, to);
            float angle = Vector3.Angle(from, to);
            return AngleAxis(angle, axis.Normalized);
        }

        public static Quaternion LookRotation(Vector3 forward, Vector3 up)
        {
            forward.Normalize();
            Vector3 vector = forward.Normalized;
            Vector3 vector2 = Vector3.Cross(up, vector).Normalized;
            Vector3 vector3 = Vector3.Cross(vector, vector2);
            float m00 = vector2.X, m01 = vector2.Y, m02 = vector2.Z;
            float m10 = vector3.X, m11 = vector3.Y, m12 = vector3.Z;
            float m20 = vector.X,  m21 = vector.Y,  m22 = vector.Z;

            float num8 = m00 + m11 + m22;
            if (num8 > 0f)
            {
                float num = MathF.Sqrt(num8 + 1f);
                float inv = 0.5f / num;
                return new Quaternion((m12 - m21) * inv, (m20 - m02) * inv, (m01 - m10) * inv, num * 0.5f);
            }
            if (m00 >= m11 && m00 >= m22)
            {
                float num7 = MathF.Sqrt(1f + m00 - m11 - m22);
                float inv = 0.5f / num7;
                return new Quaternion(0.5f * num7, (m01 + m10) * inv, (m02 + m20) * inv, (m12 - m21) * inv);
            }
            if (m11 > m22)
            {
                float num6 = MathF.Sqrt(1f + m11 - m00 - m22);
                float inv = 0.5f / num6;
                return new Quaternion((m10 + m01) * inv, 0.5f * num6, (m21 + m12) * inv, (m20 - m02) * inv);
            }
            float num5 = MathF.Sqrt(1f + m22 - m00 - m11);
            float inv2 = 0.5f / num5;
            return new Quaternion((m20 + m02) * inv2, (m21 + m12) * inv2, 0.5f * num5, (m01 - m10) * inv2);
        }

        public static Quaternion Slerp(Quaternion a, Quaternion b, float t)
            => SlerpUnclamped(a, b, Math.Clamp(t, 0f, 1f));

        public static Quaternion SlerpUnclamped(Quaternion a, Quaternion b, float t)
        {
            if (a.LengthSquared == 0f)
                return b.LengthSquared == 0f ? Identity : b;
            if (b.LengthSquared == 0f)
                return a;

            float cosHalfAngle = a.W * b.W + Vector3.Dot(a.XYZ, b.XYZ);
            if (cosHalfAngle >= 1f || cosHalfAngle <= -1f)
                return a;

            if (cosHalfAngle < 0f)
            {
                b.XYZ = -b.XYZ;
                b.W = -b.W;
                cosHalfAngle = -cosHalfAngle;
            }

            float blendA, blendB;
            if (cosHalfAngle < 0.99f)
            {
                float halfAngle = MathF.Acos(cosHalfAngle);
                float sinHalfAngle = MathF.Sin(halfAngle);
                float invSin = 1f / sinHalfAngle;
                blendA = MathF.Sin(halfAngle * (1f - t)) * invSin;
                blendB = MathF.Sin(halfAngle * t) * invSin;
            }
            else
            {
                blendA = 1f - t;
                blendB = t;
            }

            Quaternion result = new(blendA * a.XYZ + blendB * b.XYZ, blendA * a.W + blendB * b.W);
            if (result.LengthSquared > 0f)
            {
                result.Normalize();
                return result;
            }
            return Identity;
        }

        public override int GetHashCode() => HashCode.Combine(W, X, Y, Z);
        public override bool Equals(object? obj) => obj is Quaternion other && Equals(other);
        public bool Equals(Quaternion right) => X == right.X && Y == right.Y && Z == right.Z && W == right.W;
        public static bool operator ==(Quaternion left, Quaternion right) => left.Equals(right);
        public static bool operator !=(Quaternion left, Quaternion right) => !(left == right);
    }
}
