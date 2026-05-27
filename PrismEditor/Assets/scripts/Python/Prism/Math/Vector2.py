from typing import Union
from pyglm import glm


class Vector2(glm.vec2):
    Zero: "Vector2"
    One: "Vector2"
    Up: "Vector2"
    Down: "Vector2"
    Left: "Vector2"
    Right: "Vector2"

    def __init__(self, *args):
        if len(args) == 0:
            super().__init__(0.0, 0.0)
        elif len(args) == 1:
            a = args[0]
            if isinstance(a, (int, float)):
                super().__init__(float(a), float(a))
            else:
                super().__init__(a.x, a.y)
        elif len(args) == 2:
            super().__init__(float(args[0]), float(args[1]))
        else:
            super().__init__(*args)

    @property
    def Magnitude(self) -> float:
        return glm.length(self)

    @property
    def SqrMagnitude(self) -> float:
        return glm.length2(self)

    @property
    def Normalized(self) -> "Vector2":
        return Vector2(glm.normalize(self))

    def Normalize(self) -> None:
        n = glm.normalize(self)
        self.x, self.y = n.x, n.y

    def Clamp(self, min_val: "Vector2", max_val: "Vector2") -> None:
        c = glm.clamp(self, min_val, max_val)
        self.x, self.y = c.x, c.y


Vector2.Zero = Vector2(0.0, 0.0)
Vector2.One = Vector2(1.0, 1.0)
Vector2.Up = Vector2(0.0, 1.0)
Vector2.Down = Vector2(0.0, -1.0)
Vector2.Left = Vector2(-1.0, 0.0)
Vector2.Right = Vector2(1.0, 0.0)
