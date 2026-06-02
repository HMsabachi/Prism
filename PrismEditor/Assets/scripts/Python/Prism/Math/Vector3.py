from typing import Union
from pyglm import glm
from Prism.Math.Vector2 import Vector2


class Vector3(glm.vec3):
    Zero: "Vector3"
    One: "Vector3"
    Up: "Vector3"
    Down: "Vector3"
    Left: "Vector3"
    Right: "Vector3"
    Forward: "Vector3"
    Back: "Vector3"

    def __init__(self, *args):
        if len(args) == 0:
            super().__init__(0.0, 0.0, 0.0)
        elif len(args) == 1:
            a = args[0]
            if isinstance(a, Vector2):
                super().__init__(a.x, a.y, 0.0)
            elif hasattr(a, 'w'):
                super().__init__(a.x, a.y, a.z)
            elif isinstance(a, (int, float)):
                super().__init__(float(a), float(a), float(a))
            else:
                super().__init__(a.x, a.y, a.z)
        elif len(args) == 2:
            super().__init__(args[0].x, args[0].y, float(args[1]))
        elif len(args) == 3:
            super().__init__(float(args[0]), float(args[1]), float(args[2]))
        else:
            super().__init__(*args)

    @property
    def Magnitude(self) -> float:
        return glm.length(self)

    @property
    def SqrMagnitude(self) -> float:
        return glm.length2(self)

    @property
    def Normalized(self) -> "Vector3":
        return Vector3(glm.normalize(self))

    @property
    def XY(self) -> "Vector2":
        return Vector2(self.x, self.y)

    @XY.setter
    def XY(self, v: "Vector2") -> None:
        self.x, self.y = v.x, v.y

    @property
    def XZ(self) -> "Vector2":
        return Vector2(self.x, self.z)

    @XZ.setter
    def XZ(self, v: "Vector2") -> None:
        self.x, self.z = v.x, v.y

    @property
    def YZ(self) -> "Vector2":
        return Vector2(self.y, self.z)

    @YZ.setter
    def YZ(self, v: "Vector2") -> None:
        self.y, self.z = v.x, v.y

    def Normalize(self) -> None:
        n = glm.normalize(self)
        self.x, self.y, self.z = n.x, n.y, n.z

    def Clamp(self, min_val: "Vector3", max_val: "Vector3") -> None:
        c = glm.clamp(self, min_val, max_val)
        self.x, self.y, self.z = c.x, c.y, c.z


Vector3.Zero = Vector3(0.0, 0.0, 0.0)
Vector3.One = Vector3(1.0, 1.0, 1.0)
Vector3.Up = Vector3(0.0, 1.0, 0.0)
Vector3.Down = Vector3(0.0, -1.0, 0.0)
Vector3.Left = Vector3(-1.0, 0.0, 0.0)
Vector3.Right = Vector3(1.0, 0.0, 0.0)
Vector3.Forward = Vector3(0.0, 0.0, -1.0)
Vector3.Back = Vector3(0.0, 0.0, 1.0)
