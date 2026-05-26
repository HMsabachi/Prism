from pyglm import glm


class Vector4(glm.vec4):
    """4D vector backed by PyGLM for native performance."""

    @property
    def magnitude(self):
        return glm.length(self)

    @property
    def sqrMagnitude(self):
        return glm.length2(self)

    @property
    def normalized(self):
        return Vector4(glm.normalize(self))

    def Normalize(self):
        n = glm.normalize(self)
        self.x, self.y, self.z, self.w = n.x, n.y, n.z, n.w

    def Clamp(self, min, max):
        c = glm.clamp(self, min, max)
        self.x, self.y, self.z, self.w = c.x, c.y, c.z, c.w

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
            return Vector4(target)
        return Vector4(current + diff / dist * maxDistanceDelta)

    @staticmethod
    def ClampMagnitude(vector, maxLength):
        if glm.length2(vector) > maxLength * maxLength:
            return Vector4(glm.normalize(vector) * maxLength)
        return Vector4(vector)


Vector4.Zero = Vector4(0, 0, 0, 0)
Vector4.One = Vector4(1, 1, 1, 1)
