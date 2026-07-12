import math
from pyglm import glm
from Prism.Math.Vector3 import Vector3
from Prism.Math.Vector4 import Vector4


class Matrix4(glm.mat4):
    def __init__(self, *args):
        if len(args) == 0:
            super().__init__(1.0)
        elif len(args) == 1 and isinstance(args[0], (int, float)):
            super().__init__(args[0])
        else:
            super().__init__(*args)

    @staticmethod
    def Identity() -> "Matrix4":
        return Matrix4(1.0)

    @staticmethod
    def Zero() -> "Matrix4":
        return Matrix4(0.0)

    @property
    def Translation(self) -> "Vector3":
        return Vector3(self[3][0], self[3][1], self[3][2])

    @Translation.setter
    def Translation(self, value: "Vector3") -> None:
        self[3][0] = value.x
        self[3][1] = value.y
        self[3][2] = value.z

    def GetColumn(self, column: int) -> "Vector4":
        return Vector4(self[column][0], self[column][1], self[column][2], self[column][3])

    def SetColumn(self, column: int, value: "Vector4") -> None:
        self[column][0] = value.x
        self[column][1] = value.y
        self[column][2] = value.z
        self[column][3] = value.w

    def GetRow(self, row: int) -> "Vector4":
        return Vector4(self[0][row], self[1][row], self[2][row], self[3][row])

    def SetRow(self, row: int, value: "Vector4") -> None:
        self[0][row] = value.x
        self[1][row] = value.y
        self[2][row] = value.z
        self[3][row] = value.w

    def MultiplyPoint(self, point: "Vector3") -> "Vector3":
        x = self[0][0] * point.x + self[1][0] * point.y + self[2][0] * point.z + self[3][0]
        y = self[0][1] * point.x + self[1][1] * point.y + self[2][1] * point.z + self[3][1]
        z = self[0][2] * point.x + self[1][2] * point.y + self[2][2] * point.z + self[3][2]
        w = self[0][3] * point.x + self[1][3] * point.y + self[2][3] * point.z + self[3][3]
        if abs(w) > 1e-7:
            return Vector3(x / w, y / w, z / w)
        return Vector3(x, y, z)

    def MultiplyPoint3x4(self, point: "Vector3") -> "Vector3":
        return Vector3(
            self[0][0] * point.x + self[1][0] * point.y + self[2][0] * point.z + self[3][0],
            self[0][1] * point.x + self[1][1] * point.y + self[2][1] * point.z + self[3][1],
            self[0][2] * point.x + self[1][2] * point.y + self[2][2] * point.z + self[3][2])

    def MultiplyVector(self, vector: "Vector3") -> "Vector3":
        return Vector3(
            self[0][0] * vector.x + self[1][0] * vector.y + self[2][0] * vector.z,
            self[0][1] * vector.x + self[1][1] * vector.y + self[2][1] * vector.z,
            self[0][2] * vector.x + self[1][2] * vector.y + self[2][2] * vector.z)

    @property
    def Determinant(self) -> float:
        return glm.determinant(self)

    @property
    def Inverse(self) -> "Matrix4":
        try:
            return Matrix4(glm.inverse(self))
        except Exception:
            return Matrix4.Zero()

    def Invert(self) -> None:
        inv = self.Inverse
        for col in range(4):
            for row in range(4):
                self[col][row] = inv[col][row]

    @property
    def Transpose(self) -> "Matrix4":
        return Matrix4(glm.transpose(self))

    @staticmethod
    def Translate(translation: "Vector3") -> "Matrix4":
        result = Matrix4.Identity()
        result[3][0] = translation.x
        result[3][1] = translation.y
        result[3][2] = translation.z
        return result

    @staticmethod
    def Scale(scale) -> "Matrix4":
        if isinstance(scale, Vector3):
            sx, sy, sz = scale.x, scale.y, scale.z
        else:
            sx = sy = sz = float(scale)
        result = Matrix4.Zero()
        result[0][0] = sx
        result[1][1] = sy
        result[2][2] = sz
        result[3][3] = 1.0
        return result

    @staticmethod
    def TRS(pos: "Vector3", eulerAngles: "Vector3", scale: "Vector3") -> "Matrix4":
        sx = math.sin(math.radians(eulerAngles.x))
        cx = math.cos(math.radians(eulerAngles.x))
        sy = math.sin(math.radians(eulerAngles.y))
        cy = math.cos(math.radians(eulerAngles.y))
        sz = math.sin(math.radians(eulerAngles.z))
        cz = math.cos(math.radians(eulerAngles.z))
        r00 = cy * cz + sx * sy * sz
        r01 = cz * sx * sy - cy * sz
        r02 = cx * sy
        r10 = cx * sz
        r11 = cx * cz
        r12 = -sx
        r20 = cy * sx * sz - cz * sy
        r21 = cy * cz * sx + sy * sz
        r22 = cx * cy
        result = Matrix4.Zero()
        result[0][0] = scale.x * r00; result[1][0] = scale.y * r01; result[2][0] = scale.z * r02; result[3][0] = pos.x
        result[0][1] = scale.x * r10; result[1][1] = scale.y * r11; result[2][1] = scale.z * r12; result[3][1] = pos.y
        result[0][2] = scale.x * r20; result[1][2] = scale.y * r21; result[2][2] = scale.z * r22; result[3][2] = pos.z
        result[0][3] = 0.0; result[1][3] = 0.0; result[2][3] = 0.0; result[3][3] = 1.0
        return result
