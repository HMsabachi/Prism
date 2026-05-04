using System;
using System.Runtime.InteropServices;

namespace Prism
{
    [StructLayout(LayoutKind.Explicit)]
    public struct Matrix4 : IEquatable<Matrix4>
    {
        // Column-major layout: D{row}{col}
        // Consecutive in memory = same column, different rows
        // Translation is stored in column 3: D03 (x), D13 (y), D23 (z)
        [FieldOffset(0)] public float D00;
        [FieldOffset(4)] public float D10;
        [FieldOffset(8)] public float D20;
        [FieldOffset(12)] public float D30;
        [FieldOffset(16)] public float D01;
        [FieldOffset(20)] public float D11;
        [FieldOffset(24)] public float D21;
        [FieldOffset(28)] public float D31;
        [FieldOffset(32)] public float D02;
        [FieldOffset(36)] public float D12;
        [FieldOffset(40)] public float D22;
        [FieldOffset(44)] public float D32;
        [FieldOffset(48)] public float D03;
        [FieldOffset(52)] public float D13;
        [FieldOffset(56)] public float D23;
        [FieldOffset(60)] public float D33;

        public Matrix4(float value)
        {
            D00 = value; D10 = 0f; D20 = 0f; D30 = 0f;
            D01 = 0f; D11 = value; D21 = 0f; D31 = 0f;
            D02 = 0f; D12 = 0f; D22 = value; D32 = 0f;
            D03 = 0f; D13 = 0f; D23 = 0f; D33 = value;
        }

        public static Matrix4 Identity => new(1f);
        public static Matrix4 Zero => new(0f);

        public Vector3 Translation
        {
            get => new(D03, D13, D23);
            set { D03 = value.X; D13 = value.Y; D23 = value.Z; }
        }

        public float this[int row, int col]
        {
            get => this[col * 4 + row];
            set => this[col * 4 + row] = value;
        }

        public float this[int index]
        {
            get => index switch
            {
                0 => D00,  1 => D10,  2 => D20,  3 => D30,
                4 => D01,  5 => D11,  6 => D21,  7 => D31,
                8 => D02,  9 => D12, 10 => D22, 11 => D32,
                12 => D03, 13 => D13, 14 => D23, 15 => D33,
                _ => throw new IndexOutOfRangeException()
            };
            set
            {
                switch (index)
                {
                    case 0: D00 = value; break; case 1: D10 = value; break;
                    case 2: D20 = value; break; case 3: D30 = value; break;
                    case 4: D01 = value; break; case 5: D11 = value; break;
                    case 6: D21 = value; break; case 7: D31 = value; break;
                    case 8: D02 = value; break; case 9: D12 = value; break;
                    case 10: D22 = value; break; case 11: D32 = value; break;
                    case 12: D03 = value; break; case 13: D13 = value; break;
                    case 14: D23 = value; break; case 15: D33 = value; break;
                    default: throw new IndexOutOfRangeException();
                }
            }
        }

        // Column access (col 0-3, each is 4 floats consecutive in memory)
        public Vector4 GetColumn(int column)
        {
            int o = column * 4;
            return new Vector4(this[o], this[o + 1], this[o + 2], this[o + 3]);
        }

        public void SetColumn(int column, Vector4 value)
        {
            int o = column * 4;
            this[o] = value.X; this[o + 1] = value.Y;
            this[o + 2] = value.Z; this[o + 3] = value.W;
        }

        // Row access (elements of a row are spaced 4 apart in column-major)
        public Vector4 GetRow(int row)
            => new(this[row, 0], this[row, 1], this[row, 2], this[row, 3]);

        public void SetRow(int row, Vector4 value)
        {
            this[row, 0] = value.X; this[row, 1] = value.Y;
            this[row, 2] = value.Z; this[row, 3] = value.W;
        }

        // Transform point (w=1), with perspective divide for projection matrices
        public Vector3 MultiplyPoint(Vector3 point)
        {
            float x = D00 * point.X + D01 * point.Y + D02 * point.Z + D03;
            float y = D10 * point.X + D11 * point.Y + D12 * point.Z + D13;
            float z = D20 * point.X + D21 * point.Y + D22 * point.Z + D23;
            float w = D30 * point.X + D31 * point.Y + D32 * point.Z + D33;
            if (MathF.Abs(w) > 1e-7f)
            { x /= w; y /= w; z /= w; }
            return new Vector3(x, y, z);
        }

        // Transform point (w=1), no divide — for affine (TRS) matrices only
        public Vector3 MultiplyPoint3x4(Vector3 point)
        {
            return new Vector3(
                D00 * point.X + D01 * point.Y + D02 * point.Z + D03,
                D10 * point.X + D11 * point.Y + D12 * point.Z + D13,
                D20 * point.X + D21 * point.Y + D22 * point.Z + D23
            );
        }

        // Transform direction (w=0), ignores translation
        public Vector3 MultiplyVector(Vector3 vector)
        {
            return new Vector3(
                D00 * vector.X + D01 * vector.Y + D02 * vector.Z,
                D10 * vector.X + D11 * vector.Y + D12 * vector.Z,
                D20 * vector.X + D21 * vector.Y + D22 * vector.Z
            );
        }

        public float Determinant
        {
            get
            {
                float b00 = D00 * D11 - D01 * D10;
                float b01 = D00 * D12 - D02 * D10;
                float b02 = D00 * D13 - D03 * D10;
                float b03 = D01 * D12 - D02 * D11;
                float b04 = D01 * D13 - D03 * D11;
                float b05 = D02 * D13 - D03 * D12;
                float b06 = D20 * D31 - D21 * D30;
                float b07 = D20 * D32 - D22 * D30;
                float b08 = D20 * D33 - D23 * D30;
                float b09 = D21 * D32 - D22 * D31;
                float b10 = D21 * D33 - D23 * D31;
                float b11 = D22 * D33 - D23 * D32;
                return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
            }
        }

        public Matrix4 Inverse
        {
            get
            {
                float b00 = D00 * D11 - D01 * D10;
                float b01 = D00 * D12 - D02 * D10;
                float b02 = D00 * D13 - D03 * D10;
                float b03 = D01 * D12 - D02 * D11;
                float b04 = D01 * D13 - D03 * D11;
                float b05 = D02 * D13 - D03 * D12;
                float b06 = D20 * D31 - D21 * D30;
                float b07 = D20 * D32 - D22 * D30;
                float b08 = D20 * D33 - D23 * D30;
                float b09 = D21 * D32 - D22 * D31;
                float b10 = D21 * D33 - D23 * D31;
                float b11 = D22 * D33 - D23 * D32;

                float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
                if (MathF.Abs(det) < 1e-12f)
                    return Zero;

                float invDet = 1f / det;
                return new Matrix4(0f)
                {
                    D00 = (D11 * b11 - D12 * b10 + D13 * b09) * invDet,
                    D01 = (D02 * b10 - D01 * b11 - D03 * b09) * invDet,
                    D02 = (D31 * b05 - D32 * b04 + D33 * b03) * invDet,
                    D03 = (D22 * b04 - D21 * b05 - D23 * b03) * invDet,
                    D10 = (D12 * b08 - D10 * b11 - D13 * b07) * invDet,
                    D11 = (D00 * b11 - D02 * b08 + D03 * b07) * invDet,
                    D12 = (D32 * b02 - D30 * b05 - D33 * b01) * invDet,
                    D13 = (D20 * b05 - D22 * b02 + D23 * b01) * invDet,
                    D20 = (D10 * b10 - D11 * b08 + D13 * b06) * invDet,
                    D21 = (D01 * b08 - D00 * b10 - D03 * b06) * invDet,
                    D22 = (D30 * b04 - D31 * b02 + D33 * b00) * invDet,
                    D23 = (D21 * b02 - D20 * b04 - D23 * b00) * invDet,
                    D30 = (D11 * b07 - D10 * b09 - D12 * b06) * invDet,
                    D31 = (D00 * b09 - D01 * b07 + D02 * b06) * invDet,
                    D32 = (D31 * b01 - D30 * b03 - D32 * b00) * invDet,
                    D33 = (D20 * b03 - D21 * b01 + D22 * b00) * invDet,
                };
            }
        }

        public void Invert() => this = Inverse;

        public Matrix4 Transpose
        {
            get
            {
                Matrix4 r = Zero;
                r.D00 = D00; r.D01 = D10; r.D02 = D20; r.D03 = D30;
                r.D10 = D01; r.D11 = D11; r.D12 = D21; r.D13 = D31;
                r.D20 = D02; r.D21 = D12; r.D22 = D22; r.D23 = D32;
                r.D30 = D03; r.D31 = D13; r.D32 = D23; r.D33 = D33;
                return r;
            }
        }

        public static Matrix4 Translate(Vector3 translation)
        {
            Matrix4 result = Identity;
            result.D03 = translation.X;
            result.D13 = translation.Y;
            result.D23 = translation.Z;
            return result;
        }

        public static Matrix4 Scale(Vector3 scale)
        {
            Matrix4 result = Zero;
            result.D00 = scale.X;
            result.D11 = scale.Y;
            result.D22 = scale.Z;
            result.D33 = 1f;
            return result;
        }

        public static Matrix4 Scale(float scale)
            => Scale(new Vector3(scale, scale, scale));

        // Create TRS matrix from position, euler angles (degrees) and scale
        // Uses Y-X-Z rotation order (matching Unity's convention)
        public static Matrix4 TRS(Vector3 pos, Vector3 eulerAngles, Vector3 scale)
        {
            float sx = MathF.Sin(eulerAngles.X * MathF.PI / 180f);
            float cx = MathF.Cos(eulerAngles.X * MathF.PI / 180f);
            float sy = MathF.Sin(eulerAngles.Y * MathF.PI / 180f);
            float cy = MathF.Cos(eulerAngles.Y * MathF.PI / 180f);
            float sz = MathF.Sin(eulerAngles.Z * MathF.PI / 180f);
            float cz = MathF.Cos(eulerAngles.Z * MathF.PI / 180f);
            // R = Rz * Rx * Ry
            float r00 = cy * cz + sx * sy * sz;
            float r01 = cz * sx * sy - cy * sz;
            float r02 = cx * sy;
            float r10 = cx * sz;
            float r11 = cx * cz;
            float r12 = -sx;
            float r20 = cy * sx * sz - cz * sy;
            float r21 = cy * cz * sx + sy * sz;
            float r22 = cx * cy;

            Matrix4 result = Zero;
            result.D00 = scale.X * r00; result.D01 = scale.Y * r01; result.D02 = scale.Z * r02; result.D03 = pos.X;
            result.D10 = scale.X * r10; result.D11 = scale.Y * r11; result.D12 = scale.Z * r12; result.D13 = pos.Y;
            result.D20 = scale.X * r20; result.D21 = scale.Y * r21; result.D22 = scale.Z * r22; result.D23 = pos.Z;
            result.D30 = 0f; result.D31 = 0f; result.D32 = 0f; result.D33 = 1f;
            return result;
        }

        public static Matrix4 operator *(Matrix4 left, Matrix4 right)
        {
            Matrix4 r = Zero;
            r.D00 = left.D00 * right.D00 + left.D01 * right.D10 + left.D02 * right.D20 + left.D03 * right.D30;
            r.D10 = left.D10 * right.D00 + left.D11 * right.D10 + left.D12 * right.D20 + left.D13 * right.D30;
            r.D20 = left.D20 * right.D00 + left.D21 * right.D10 + left.D22 * right.D20 + left.D23 * right.D30;
            r.D30 = left.D30 * right.D00 + left.D31 * right.D10 + left.D32 * right.D20 + left.D33 * right.D30;
            r.D01 = left.D00 * right.D01 + left.D01 * right.D11 + left.D02 * right.D21 + left.D03 * right.D31;
            r.D11 = left.D10 * right.D01 + left.D11 * right.D11 + left.D12 * right.D21 + left.D13 * right.D31;
            r.D21 = left.D20 * right.D01 + left.D21 * right.D11 + left.D22 * right.D21 + left.D23 * right.D31;
            r.D31 = left.D30 * right.D01 + left.D31 * right.D11 + left.D32 * right.D21 + left.D33 * right.D31;
            r.D02 = left.D00 * right.D02 + left.D01 * right.D12 + left.D02 * right.D22 + left.D03 * right.D32;
            r.D12 = left.D10 * right.D02 + left.D11 * right.D12 + left.D12 * right.D22 + left.D13 * right.D32;
            r.D22 = left.D20 * right.D02 + left.D21 * right.D12 + left.D22 * right.D22 + left.D23 * right.D32;
            r.D32 = left.D30 * right.D02 + left.D31 * right.D12 + left.D32 * right.D22 + left.D33 * right.D32;
            r.D03 = left.D00 * right.D03 + left.D01 * right.D13 + left.D02 * right.D23 + left.D03 * right.D33;
            r.D13 = left.D10 * right.D03 + left.D11 * right.D13 + left.D12 * right.D23 + left.D13 * right.D33;
            r.D23 = left.D20 * right.D03 + left.D21 * right.D13 + left.D22 * right.D23 + left.D23 * right.D33;
            r.D33 = left.D30 * right.D03 + left.D31 * right.D13 + left.D32 * right.D23 + left.D33 * right.D33;
            return r;
        }

        public static Vector3 operator *(Matrix4 matrix, Vector3 point)
            => matrix.MultiplyPoint3x4(point);

        public static Vector4 operator *(Matrix4 matrix, Vector4 vector)
        {
            return new Vector4(
                matrix.D00 * vector.X + matrix.D01 * vector.Y + matrix.D02 * vector.Z + matrix.D03 * vector.W,
                matrix.D10 * vector.X + matrix.D11 * vector.Y + matrix.D12 * vector.Z + matrix.D13 * vector.W,
                matrix.D20 * vector.X + matrix.D21 * vector.Y + matrix.D22 * vector.Z + matrix.D23 * vector.W,
                matrix.D30 * vector.X + matrix.D31 * vector.Y + matrix.D32 * vector.Z + matrix.D33 * vector.W
            );
        }

        public static bool operator ==(Matrix4 a, Matrix4 b)
        {
            return a.D00 == b.D00 && a.D10 == b.D10 && a.D20 == b.D20 && a.D30 == b.D30
                && a.D01 == b.D01 && a.D11 == b.D11 && a.D21 == b.D21 && a.D31 == b.D31
                && a.D02 == b.D02 && a.D12 == b.D12 && a.D22 == b.D22 && a.D32 == b.D32
                && a.D03 == b.D03 && a.D13 == b.D13 && a.D23 == b.D23 && a.D33 == b.D33;
        }

        public static bool operator !=(Matrix4 a, Matrix4 b) => !(a == b);

        public override bool Equals(object? obj)
            => obj is Matrix4 other && Equals(other);

        public override int GetHashCode()
        {
            HashCode h = new();
            h.Add(D00); h.Add(D10); h.Add(D20); h.Add(D30);
            h.Add(D01); h.Add(D11); h.Add(D21); h.Add(D31);
            h.Add(D02); h.Add(D12); h.Add(D22); h.Add(D32);
            h.Add(D03); h.Add(D13); h.Add(D23); h.Add(D33);
            return h.ToHashCode();
        }

        public bool Equals(Matrix4 other) => this == other;

        public override string ToString()
        {
            return $"{D00:F2}\t{D01:F2}\t{D02:F2}\t{D03:F2}\n"
                 + $"{D10:F2}\t{D11:F2}\t{D12:F2}\t{D13:F2}\n"
                 + $"{D20:F2}\t{D21:F2}\t{D22:F2}\t{D23:F2}\n"
                 + $"{D30:F2}\t{D31:F2}\t{D32:F2}\t{D33:F2}";
        }

        public void DebugPrint()
            => Log.Trace($"Matrix4:\n{this}");
    }
}
