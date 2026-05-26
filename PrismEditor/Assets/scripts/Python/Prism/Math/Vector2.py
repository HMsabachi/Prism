from pyglm import glm


class Vector2(glm.vec2):
    """2D vector backed by PyGLM for native performance."""

    @property
    def Magnitude(self):
        return glm.length(self)

    @property
    def SqrMagnitude(self):
        return glm.length2(self)

    @property
    def Normalized(self):
        return Vector2(glm.normalize(self))

    def Normalize(self):
        n = glm.normalize(self)
        self.x, self.y = n.x, n.y

    def Clamp(self, min, max):
        c = glm.clamp(self, min, max)
        self.x, self.y = c.x, c.y

    @staticmethod
    def Distance(a, b):
        return glm.distance(a, b)

    @staticmethod
    def Dot(a, b):
        return glm.dot(a, b)

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
            return Vector2(target)
        return Vector2(current + diff / dist * maxDistanceDelta)

    @staticmethod
    def Scale(a, b):
        return a * b

    @staticmethod
    def Reflect(inDirection, inNormal):
        return Vector2(glm.reflect(inDirection, inNormal))

    @staticmethod
    def Perpendicular(inDirection):
        return Vector2(-inDirection.y, inDirection.x)

    @staticmethod
    def Clamp(value, min, max):
        return Vector2(glm.clamp(value, min, max))

    @staticmethod
    def EpsilonEquals(a, b):
        diff = a - b
        return glm.length2(diff) < 1e-12


Vector2.Zero = Vector2(0, 0)
Vector2.One = Vector2(1, 1)
Vector2.Up = Vector2(0, 1)
Vector2.Down = Vector2(0, -1)
Vector2.Left = Vector2(-1, 0)
Vector2.Right = Vector2(1, 0)
