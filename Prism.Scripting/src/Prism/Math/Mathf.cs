using System;

namespace Prism
{
    public static class Mathf
    {
        public const float PI = MathF.PI;
        public const float Epsilon = 1.17549435E-38f;

        public static float Deg2Rad => PI / 180f;
        public static float Rad2Deg => 180f / PI;

        public static float Sin(float f) => MathF.Sin(f);
        public static float Cos(float f) => MathF.Cos(f);
        public static float Tan(float f) => MathF.Tan(f);
        public static float Asin(float f) => MathF.Asin(f);
        public static float Acos(float f) => MathF.Acos(f);
        public static float Atan(float f) => MathF.Atan(f);
        public static float Atan2(float y, float x) => MathF.Atan2(y, x);
        public static float Sqrt(float f) => MathF.Sqrt(f);
        public static float Abs(float f) => MathF.Abs(f);
        public static int Abs(int value) => Math.Abs(value);
        public static float Min(float a, float b) => MathF.Min(a, b);
        public static float Max(float a, float b) => MathF.Max(a, b);
        public static float Pow(float f, float p) => MathF.Pow(f, p);
        public static float Exp(float power) => MathF.Exp(power);
        public static float Log(float f, float p) => MathF.Log(f, p);
        public static float Log(float f) => MathF.Log(f);
        public static float Log10(float f) => MathF.Log10(f);
        public static float Floor(float f) => MathF.Floor(f);
        public static float Ceil(float f) => MathF.Ceiling(f);
        public static float Round(float f) => MathF.Round(f);
        public static int RoundToInt(float f) => (int)MathF.Round(f);
        public static int FloorToInt(float f) => (int)MathF.Floor(f);
        public static int CeilToInt(float f) => (int)MathF.Ceiling(f);
        public static float Sign(float f) => f >= 0f ? 1f : -1f;

        public static float Clamp(float value, float min, float max)
            => Math.Clamp(value, min, max);

        public static int Clamp(int value, int min, int max)
            => Math.Clamp(value, min, max);

        public static float Clamp01(float value)
            => Math.Clamp(value, 0f, 1f);

        public static float Lerp(float a, float b, float t)
            => a + (b - a) * Clamp01(t);

        public static float LerpUnclamped(float a, float b, float t)
            => a + (b - a) * t;

        public static float InverseLerp(float a, float b, float value)
        {
            if (a != b)
                return Clamp01((value - a) / (b - a));
            return 0f;
        }

        public static float MoveTowards(float current, float target, float maxDelta)
        {
            if (Abs(target - current) <= maxDelta)
                return target;
            return current + Sign(target - current) * maxDelta;
        }

        public static float Repeat(float t, float length)
            => Clamp(t - Floor(t / length) * length, 0f, length);

        public static float PingPong(float t, float length)
        {
            t = Repeat(t, length * 2f);
            return length - Abs(t - length);
        }

        public static float SmoothStep(float from, float to, float t)
        {
            t = Clamp01(t);
            t = t * t * (3f - 2f * t);
            return from + (to - from) * t;
        }

        public static bool Approximately(float a, float b)
            => Abs(b - a) < Max(1E-6f * Max(Abs(a), Abs(b)), Epsilon * 8f);

        public static float DeltaAngle(float current, float target)
        {
            float delta = Repeat(target - current, 360f);
            if (delta > 180f)
                delta -= 360f;
            return delta;
        }

        public static float MoveTowardsAngle(float current, float target, float maxDelta)
        {
            float delta = DeltaAngle(current, target);
            if (-maxDelta < delta && delta < maxDelta)
                return target;
            target = current + delta;
            return MoveTowards(current, target, maxDelta);
        }

        public static float SmoothDamp(float current, float target, ref float currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
        {
            smoothTime = Max(0.0001f, smoothTime);
            float omega = 2f / smoothTime;
            float x = omega * deltaTime;
            float exp = 1f / (1f + x + 0.48f * x * x + 0.235f * x * x * x);
            float change = target - current;
            float maxChange = maxSpeed * smoothTime;
            change = Clamp(change, -maxChange, maxChange);
            float temp = (currentVelocity + omega * change) * deltaTime;
            currentVelocity = (currentVelocity - omega * temp) * exp;
            return current + change * exp;
        }

        public static float SmoothDamp(float current, float target, ref float currentVelocity, float smoothTime)
            => SmoothDamp(current, target, ref currentVelocity, smoothTime, float.PositiveInfinity, Time.DeltaTime);

        public static float SmoothDampAngle(float current, float target, ref float currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
            => SmoothDamp(current, target + 360f, ref currentVelocity, smoothTime, maxSpeed, deltaTime);

        public static float SmoothDampAngle(float current, float target, ref float currentVelocity, float smoothTime)
            => SmoothDampAngle(current, target, ref currentVelocity, smoothTime, float.PositiveInfinity, Time.DeltaTime);
    }
}
