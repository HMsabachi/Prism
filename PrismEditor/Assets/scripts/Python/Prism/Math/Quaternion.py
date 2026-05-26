from pyglm import glm
from Prism.Math.Vector3 import Vector3


class Quaternion(glm.quat):
    """四元数，基于 PyGLM glm.quat，API 对称 C# Quaternion.cs。

    构造函数: Quaternion(x=0, y=0, z=0, w=1) 匹配 C# 参数顺序。
    """

    def __new__(cls, x=0, y=0, z=0, w=1):
        return super().__new__(cls, w, x, y, z)

    # --- 实例属性 (同 C#) ---

    @property
    def Conjugate(self):
        return Quaternion(glm.conjugate(self))

    @property
    def Length(self):
        return glm.length(self)

    @property
    def LengthSquared(self):
        return glm.length2(self)

    @property
    def XYZ(self):
        return Vector3(self.x, self.y, self.z)

    @XYZ.setter
    def XYZ(self, value):
        self.x, self.y, self.z = value.x, value.y, value.z

    # --- 实例方法 (同 C#) ---

    def Normalize(self):
        n = glm.normalize(self)
        self.x, self.y, self.z, self.w = n.x, n.y, n.z, n.w

    def EulerAngles(self):
        """返回欧拉角 (度)，Z-X-Y (Yaw-Pitch-Roll) 顺序。"""
        e = glm.eulerAngles(self)
        return Vector3(glm.degrees(e))

    # --- 运算符 (同 C#) ---

    def __mul__(self, other):
        if isinstance(other, (glm.vec3, Vector3)):
            # Quaternion * Vector3: 旋转向量
            qv = glm.vec3(self.x, self.y, self.z)
            uv = glm.cross(qv, other)
            uuv = glm.cross(qv, uv)
            result = other + (uv * self.w + uuv) * 2.0
            return Vector3(result) if isinstance(other, Vector3) else result
        # Quaternion * Quaternion: 组合旋转
        return Quaternion(super().__mul__(other))

    # --- 静态方法 (同 C#) ---

    @staticmethod
    def Euler(x, y=None, z=None):
        """从欧拉角 (度) 创建四元数。"""
        if isinstance(x, (glm.vec3, Vector3)):
            v = x
            rad = glm.radians(glm.vec3(v.x, v.y, v.z))
        else:
            rad = glm.radians(glm.vec3(x, y or 0, z or 0))
        return Quaternion(glm.quat(rad))

    @staticmethod
    def AngleAxis(angleDeg, axis):
        a = glm.normalize(glm.vec3(axis))
        return Quaternion(glm.angleAxis(glm.radians(angleDeg), a))

    @staticmethod
    def FromToRotation(from_vec, to_vec):
        f = glm.normalize(glm.vec3(from_vec))
        t = glm.normalize(glm.vec3(to_vec))
        return Quaternion(glm.rotation(f, t))

    @staticmethod
    def LookRotation(forward, up=None):
        f = glm.normalize(glm.vec3(forward))
        u = glm.normalize(glm.vec3(up)) if up is not None else glm.vec3(0, 1, 0)
        return Quaternion(glm.quatLookAt(f, u))

    @staticmethod
    def Slerp(a, b, t):
        t = max(0.0, min(1.0, float(t)))
        return Quaternion(glm.slerp(a, b, t))

    @staticmethod
    def SlerpUnclamped(a, b, t):
        return Quaternion(glm.slerp(a, b, float(t)))

    # --- 相等 (同 C#) ---

    def __eq__(self, other):
        if isinstance(other, Quaternion):
            return (abs(self.x - other.x) < 1e-8 and
                    abs(self.y - other.y) < 1e-8 and
                    abs(self.z - other.z) < 1e-8 and
                    abs(self.w - other.w) < 1e-8)
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def __hash__(self):
        return hash((self.w, self.x, self.y, self.z))


# 常量 (同 C#)
Quaternion.Identity = Quaternion(0, 0, 0, 1)
