from typing import Union
from pyglm import glm
from Prism.Math.Vector3 import Vector3


class Vector4(glm.vec4):
    Zero: "Vector4"
    One: "Vector4"

    def __init__(self, *args):
        if len(args) == 0:
            super().__init__(0.0, 0.0, 0.0, 0.0)
        elif len(args) == 1:
            a = args[0]
            if isinstance(a, Vector3):
                super().__init__(a.x, a.y, a.z, 0.0)
            elif isinstance(a, (int, float)):
                super().__init__(float(a), float(a), float(a), float(a))
            else:
                super().__init__(a.x, a.y, a.z, a.w)
        elif len(args) == 2:
            super().__init__(args[0].x, args[0].y, args[0].z, float(args[1]))
        elif len(args) == 4:
            super().__init__(float(args[0]), float(args[1]), float(args[2]), float(args[3]))
        else:
            super().__init__(*args)

    @property
    def Magnitude(self) -> float:
        return glm.length(self)

    @property
    def SqrMagnitude(self) -> float:
        return glm.length2(self)

    @property
    def Normalized(self) -> "Vector4":
        return Vector4(glm.normalize(self))

    def Normalize(self) -> None:
        n = glm.normalize(self)
        self.x, self.y, self.z, self.w = n.x, n.y, n.z, n.w

    def Clamp(self, min_val: "Vector4", max_val: "Vector4") -> None:
        c = glm.clamp(self, min_val, max_val)
        self.x, self.y, self.z, self.w = c.x, c.y, c.z, c.w


Vector4.Zero = Vector4(0.0, 0.0, 0.0, 0.0)
Vector4.One = Vector4(1.0, 1.0, 1.0, 1.0)
