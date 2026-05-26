import math


class Mathf:
    PI = math.pi
    Epsilon = 1.17549435e-38
    Deg2Rad = math.pi / 180.0
    Rad2Deg = 180.0 / math.pi

    @staticmethod
    def Sin(f): return math.sin(f)

    @staticmethod
    def Cos(f): return math.cos(f)

    @staticmethod
    def Tan(f): return math.tan(f)

    @staticmethod
    def Asin(f): return math.asin(f)

    @staticmethod
    def Acos(f): return math.acos(f)

    @staticmethod
    def Atan(f): return math.atan(f)

    @staticmethod
    def Atan2(y, x): return math.atan2(y, x)

    @staticmethod
    def Sqrt(f): return math.sqrt(f)

    @staticmethod
    def Abs(f):
        if isinstance(f, int):
            return abs(f)
        return math.fabs(f)

    @staticmethod
    def Min(a, b): return min(a, b)

    @staticmethod
    def Max(a, b): return max(a, b)

    @staticmethod
    def Pow(f, p): return math.pow(f, p)

    @staticmethod
    def Exp(power): return math.exp(power)

    @staticmethod
    def Log(f, p=None):
        if p is None:
            return math.log(f)
        return math.log(f, p)

    @staticmethod
    def Log10(f): return math.log10(f)

    @staticmethod
    def Floor(f): return math.floor(f)

    @staticmethod
    def Ceil(f): return math.ceil(f)

    @staticmethod
    def Round(f): return round(f)

    @staticmethod
    def RoundToInt(f): return int(round(f))

    @staticmethod
    def FloorToInt(f): return int(math.floor(f))

    @staticmethod
    def CeilToInt(f): return int(math.ceil(f))

    @staticmethod
    def Sign(f): return 1.0 if f >= 0.0 else -1.0

    @staticmethod
    def Clamp(value, min_val, max_val):
        if isinstance(value, int) and isinstance(min_val, int) and isinstance(max_val, int):
            return max(min_val, min(max_val, value))
        return max(float(min_val), min(float(max_val), float(value)))

    @staticmethod
    def Clamp01(value):
        return max(0.0, min(1.0, float(value)))

    @staticmethod
    def Lerp(a, b, t):
        return a + (b - a) * Mathf.Clamp01(t)

    @staticmethod
    def LerpUnclamped(a, b, t):
        return a + (b - a) * t

    @staticmethod
    def InverseLerp(a, b, value):
        if a != b:
            return Mathf.Clamp01((value - a) / (b - a))
        return 0.0

    @staticmethod
    def MoveTowards(current, target, maxDelta):
        if abs(target - current) <= maxDelta:
            return target
        return current + Mathf.Sign(target - current) * maxDelta

    @staticmethod
    def Repeat(t, length):
        return Mathf.Clamp(t - math.floor(t / length) * length, 0.0, length)

    @staticmethod
    def PingPong(t, length):
        t = Mathf.Repeat(t, length * 2.0)
        return length - abs(t - length)

    @staticmethod
    def SmoothStep(from_val, to_val, t):
        t = Mathf.Clamp01(t)
        t = t * t * (3.0 - 2.0 * t)
        return from_val + (to_val - from_val) * t

    @staticmethod
    def Approximately(a, b):
        return abs(b - a) < max(1e-6 * max(abs(a), abs(b)), Mathf.Epsilon * 8.0)

    @staticmethod
    def DeltaAngle(current, target):
        delta = Mathf.Repeat(target - current, 360.0)
        if delta > 180.0:
            delta -= 360.0
        return delta

    @staticmethod
    def MoveTowardsAngle(current, target, maxDelta):
        delta = Mathf.DeltaAngle(current, target)
        if -maxDelta < delta < maxDelta:
            return target
        target = current + delta
        return Mathf.MoveTowards(current, target, maxDelta)

    @staticmethod
    def SmoothDamp(current, target, currentVelocity, smoothTime, maxSpeed=float('inf'), deltaTime=None):
        smoothTime = max(1e-4, smoothTime)
        omega = 2.0 / smoothTime
        dt = deltaTime if deltaTime is not None else 1.0 / 60.0
        x = omega * dt
        exp = 1.0 / (1.0 + x + 0.48 * x * x + 0.235 * x * x * x)
        change = target - current
        maxChange = maxSpeed * smoothTime
        change = Mathf.Clamp(change, -maxChange, maxChange)
        temp = (currentVelocity + omega * change) * dt
        newVelocity = (currentVelocity - omega * temp) * exp
        return current + change * exp, newVelocity

    @staticmethod
    def SmoothDampAngle(current, target, currentVelocity, smoothTime, maxSpeed=float('inf'), deltaTime=None):
        return Mathf.SmoothDamp(current, target + 360.0, currentVelocity, smoothTime, maxSpeed, deltaTime)
