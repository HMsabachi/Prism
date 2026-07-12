import math
from pyglm import glm
from typing import Union, overload


class Mathf:
    PI: float = math.pi
    Epsilon: float = 1.17549435e-38
    Deg2Rad: float = math.pi / 180.0
    Rad2Deg: float = 180.0 / math.pi

    @staticmethod
    def Sin(f: float) -> float: return math.sin(f)
    @staticmethod
    def Cos(f: float) -> float: return math.cos(f)
    @staticmethod
    def Tan(f: float) -> float: return math.tan(f)
    @staticmethod
    def Asin(f: float) -> float: return math.asin(f)
    @staticmethod
    def Acos(f: float) -> float: return math.acos(f)
    @staticmethod
    def Atan(f: float) -> float: return math.atan(f)
    @staticmethod
    def Atan2(y: float, x: float) -> float: return math.atan2(y, x)
    @staticmethod
    def Sqrt(f: float) -> float: return math.sqrt(f)

    @staticmethod
    def Abs(f: Union[int, float]) -> float:
        return abs(f)

    @staticmethod
    def Min(a, b): return a if a < b else b
    @staticmethod
    def Max(a, b): return a if a > b else b

    @staticmethod
    def Pow(f: float, p: float) -> float: return math.pow(f, p)
    @staticmethod
    def Exp(power: float) -> float: return math.exp(power)

    @staticmethod
    def Log(f: float, p: float = None):
        if p is None: return math.log(f)
        return math.log(f, p)

    @staticmethod
    def Log10(f: float) -> float: return math.log10(f)
    @staticmethod
    def Floor(f: float) -> float: return math.floor(f)
    @staticmethod
    def Ceil(f: float) -> float: return math.ceil(f)
    @staticmethod
    def Round(f: float) -> float: return round(f)
    @staticmethod
    def RoundToInt(f: float) -> int: return int(round(f))
    @staticmethod
    def FloorToInt(f: float) -> int: return int(math.floor(f))
    @staticmethod
    def CeilToInt(f: float) -> int: return int(math.ceil(f))
    @staticmethod
    def Sign(f: float) -> float: return 1.0 if f >= 0.0 else -1.0

    # ── Clamp ──────────────────────────────────

    @staticmethod
    def Clamp(value, min_val, max_val):
        if isinstance(value, (int, float)):
            return max(min_val, min(max_val, value))
        cls = type(value)
        return cls(glm.clamp(value, min_val, max_val))

    @staticmethod
    def Clamp01(value: float) -> float:
        return max(0.0, min(1.0, float(value)))

    # ── Lerp ───────────────────────────────────

    @staticmethod
    def Lerp(a, b, t):
        t = Mathf.Clamp01(t)
        return a + (b - a) * t

    @staticmethod
    def LerpUnclamped(a, b, t):
        return a + (b - a) * t

    @staticmethod
    def InverseLerp(a: float, b: float, value: float) -> float:
        if a != b:
            return Mathf.Clamp01((value - a) / (b - a))
        return 0.0

    # ── MoveTowards ────────────────────────────

    @staticmethod
    def MoveTowards(current, target, maxDelta):
        if Mathf.Abs(target - current) <= maxDelta:
            return target
        return current + Mathf.Sign(target - current) * maxDelta

    @staticmethod
    def Repeat(t: float, length: float) -> float:
        return Mathf.Clamp(t - math.floor(t / length) * length, 0.0, length)

    @staticmethod
    def PingPong(t: float, length: float) -> float:
        t = Mathf.Repeat(t, length * 2.0)
        return length - abs(t - length)

    @staticmethod
    def SmoothStep(from_val: float, to_val: float, t: float) -> float:
        t = Mathf.Clamp01(t)
        t = t * t * (3.0 - 2.0 * t)
        return from_val + (to_val - from_val) * t

    @staticmethod
    def Approximately(a: float, b: float) -> bool:
        return abs(b - a) < max(1e-6 * max(abs(a), abs(b)), Mathf.Epsilon * 8.0)

    @staticmethod
    def DeltaAngle(current: float, target: float) -> float:
        delta = Mathf.Repeat(target - current, 360.0)
        if delta > 180.0:
            delta -= 360.0
        return delta

    @staticmethod
    def MoveTowardsAngle(current: float, target: float, maxDelta: float) -> float:
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
    def SmoothDampAngle(current: float, target: float, currentVelocity: float,
                        smoothTime: float, maxSpeed: float = float('inf'), deltaTime: float = None):
        return Mathf.SmoothDamp(current, target + 360.0, currentVelocity, smoothTime, maxSpeed, deltaTime)

    # ── Vector 静态方法 (从 Vector2/3/4 移入) ──

    @staticmethod
    def Distance(a, b) -> float:
        return glm.distance(a, b)

    @staticmethod
    def Dot(a, b) -> float:
        return glm.dot(a, b)

    @staticmethod
    def Cross(a, b):
        from Prism.Math.Vector3 import Vector3
        return Vector3(glm.cross(a, b))

    @staticmethod
    def Reflect(direction, normal):
        cls = type(direction)
        return cls(glm.reflect(direction, normal))

    @staticmethod
    def Project(vector, onNormal):
        from Prism.Math.Vector3 import Vector3
        return Vector3(glm.project(vector, onNormal))

    @staticmethod
    def ClampMagnitude(vector, maxLength):
        cls = type(vector)
        if glm.length2(vector) > maxLength * maxLength:
            return cls(glm.normalize(vector) * maxLength)
        return cls(vector)

    @staticmethod
    def Angle(from_vec, to_vec) -> float:
        return glm.degrees(glm.angle(from_vec, to_vec))

    @staticmethod
    def Scale(a, b):
        return a * b

    @staticmethod
    def Perpendicular(direction):
        from Prism.Math.Vector2 import Vector2
        return Vector2(-direction.y, direction.x)
