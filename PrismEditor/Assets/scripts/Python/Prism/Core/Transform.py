import PrismNative as _Prism
from Prism.Math.Vector3 import Vector3


class Transform:
    """纯数据容器，所有字段由 C++ 引擎填充。"""

    def __init__(self, position: Vector3, rotation: Vector3, scale: Vector3,
                 up: Vector3 = None, right: Vector3 = None, forward: Vector3 = None):
        self.Position = position
        self.Rotation = rotation
        self.Scale = scale
        self.Up = up or Vector3(0.0, 1.0, 0.0)
        self.Right = right or Vector3(1.0, 0.0, 0.0)
        self.Forward = forward or Vector3(0.0, 0.0, -1.0)
