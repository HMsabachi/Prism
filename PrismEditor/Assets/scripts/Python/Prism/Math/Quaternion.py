from pyglm import glm
from Prism.Math.Vector3 import Vector3


class Quaternion(glm.quat):
    Identity: "Quaternion"

    def __init__(self, *args):
        if len(args) == 0:
            super().__init__(1.0, 0.0, 0.0, 0.0)
        elif len(args) == 1:
            a = args[0]
            if isinstance(a, Vector3):
                rad = glm.radians(glm.vec3(a.x, a.y, a.z))
                super().__init__(glm.quat(rad))
            elif isinstance(a, glm.quat):
                super().__init__(a.w, a.x, a.y, a.z)
            else:
                super().__init__(a)
        elif len(args) == 2:
            if isinstance(args[0], Vector3):
                super().__init__(float(args[1]), args[0].x, args[0].y, args[0].z)
            else:
                super().__init__(*args)
        elif len(args) == 4:
            super().__init__(float(args[3]), float(args[0]), float(args[1]), float(args[2]))
        else:
            super().__init__(*args)

    @property
    def Conjugate(self) -> "Quaternion":
        return Quaternion(glm.conjugate(self))

    @property
    def Length(self) -> float:
        return glm.length(self)

    @property
    def LengthSquared(self) -> float:
        return glm.length2(self)

    @property
    def XYZ(self) -> "Vector3":
        return Vector3(self.x, self.y, self.z)

    @XYZ.setter
    def XYZ(self, value: "Vector3") -> None:
        self.x, self.y, self.z = value.x, value.y, value.z

    def Normalize(self) -> None:
        n = glm.normalize(self)
        self.x, self.y, self.z, self.w = n.x, n.y, n.z, n.w

    def EulerAngles(self) -> "Vector3":
        e = glm.eulerAngles(self)
        return Vector3(glm.degrees(e))

    def __mul__(self, other):
        if isinstance(other, (glm.vec3, Vector3)):
            qv = glm.vec3(self.x, self.y, self.z)
            uv = glm.cross(qv, other)
            uuv = glm.cross(qv, uv)
            result = other + (uv * self.w + uuv) * 2.0
            return Vector3(result) if isinstance(other, Vector3) else result
        return Quaternion(super().__mul__(other))

    @staticmethod
    def Euler(x, y=None, z=None):
        if isinstance(x, (glm.vec3, Vector3)):
            v = x
            rad = glm.radians(glm.vec3(v.x, v.y, v.z))
        else:
            rad = glm.radians(glm.vec3(x, y or 0, z or 0))
        return Quaternion(glm.quat(rad))

    @staticmethod
    def AngleAxis(angleDeg: float, axis: "Vector3") -> "Quaternion":
        a = glm.normalize(glm.vec3(axis))
        return Quaternion(glm.angleAxis(glm.radians(angleDeg), a))

    @staticmethod
    def FromToRotation(from_vec: "Vector3", to_vec: "Vector3") -> "Quaternion":
        f = glm.normalize(glm.vec3(from_vec))
        t = glm.normalize(glm.vec3(to_vec))
        return Quaternion(glm.rotation(f, t))

    @staticmethod
    def LookRotation(forward: "Vector3", up: "Vector3" = None) -> "Quaternion":
        f = glm.normalize(glm.vec3(forward))
        u = glm.normalize(glm.vec3(up)) if up is not None else glm.vec3(0, 1, 0)
        return Quaternion(glm.quatLookAt(f, u))

    @staticmethod
    def Slerp(a: "Quaternion", b: "Quaternion", t: float) -> "Quaternion":
        t = max(0.0, min(1.0, float(t)))
        return Quaternion(glm.slerp(a, b, t))

    @staticmethod
    def SlerpUnclamped(a: "Quaternion", b: "Quaternion", t: float) -> "Quaternion":
        return Quaternion(glm.slerp(a, b, float(t)))

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


Quaternion.Identity = Quaternion(0, 0, 0, 1)
