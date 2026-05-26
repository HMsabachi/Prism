from pyglm import glm
from Prism.Math.Vector2 import Vector2


class Vector3(glm.vec3):
    """3D vector backed by PyGLM for native performance."""

    @property
    def Magnitude(self):
        return glm.length(self)

    @property
    def SqrMagnitude(self):
        return glm.length2(self)

    @property
    def Normalized(self):
        return Vector3(glm.normalize(self))

    @property
    def XY(self):
        return Vector2(self.x, self.y)

    @XY.setter
    def XY(self, v):
        self.x, self.y = v.x, v.y

    @property
    def XZ(self):
        return Vector2(self.x, self.z)

    @XZ.setter
    def XZ(self, v):
        self.x, self.z = v.x, v.y

    @property
    def YZ(self):
        return Vector2(self.y, self.z)

    @YZ.setter
    def YZ(self, v):
        self.y, self.z = v.x, v.y

    def Normalize(self):
        n = glm.normalize(self)
        self.x, self.y, self.z = n.x, n.y, n.z

    def Clamp(self, min, max):
        c = glm.clamp(self, min, max)
        self.x, self.y, self.z = c.x, c.y, c.z

    @staticmethod
    def Distance(a, b):
        return glm.distance(a, b)

    @staticmethod
    def Dot(a, b):
        return glm.dot(a, b)

    @staticmethod
    def Cross(a, b):
        return Vector3(glm.cross(a, b))

    @staticmethod
    def Lerp(a, b, t):
        return glm.mix(a, b, glm.clamp(t, 0, 1))

    @staticmethod
    def LerpUnclamped(a, b, t):
        return glm.mix(a, b, t)

    @staticmethod
    def MoveTowards(current, target, maxDistanceDelta):
        diff = target - current
        dist = glm.length(diff)
        if dist <= maxDistanceDelta or dist == 0:
            return Vector3(target)
        return Vector3(current + diff / dist * maxDistanceDelta)

    @staticmethod
    def Angle(from_vec, to):
        return glm.degrees(glm.angle(from_vec, to))

    @staticmethod
    def Scale(a, b):
        return a * b

    @staticmethod
    def Project(vector, onNormal):
        return Vector3(glm.project(vector, onNormal))

    @staticmethod
    def Reflect(inDirection, inNormal):
        return Vector3(glm.reflect(inDirection, inNormal))

    @staticmethod
    def ClampMagnitude(vector, maxLength):
        if glm.length2(vector) > maxLength * maxLength:
            return Vector3(glm.normalize(vector) * maxLength)
        return Vector3(vector)

    @staticmethod
    def Cos(v):
        return Vector3(glm.cos(v))

    @staticmethod
    def Sin(v):
        return Vector3(glm.sin(v))

    @staticmethod
    def Clamp(value, min, max):
        return Vector3(glm.clamp(value, min, max))


Vector3.Zero = Vector3(0, 0, 0)
Vector3.One = Vector3(1, 1, 1)
Vector3.Up = Vector3(0, 1, 0)
Vector3.Down = Vector3(0, -1, 0)
Vector3.Left = Vector3(-1, 0, 0)
Vector3.Right = Vector3(1, 0, 0)
Vector3.Forward = Vector3(0, 0, 1)
Vector3.Back = Vector3(0, 0, -1)
