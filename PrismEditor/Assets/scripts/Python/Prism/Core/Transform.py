import PrismNative as _Prism
from Prism.Math.Vector3 import Vector3


class Transform:
    """Wrapper for C++ TransformComponent, providing Position/Rotation/Scale access."""

    def __init__(self, entity_id: int):
        self._entity_id = entity_id

    @property
    def position(self) -> Vector3:
        return Vector3(_Prism.GetPosition(self._entity_id))

    @position.setter
    def position(self, value):
        _Prism.SetPosition(self._entity_id, value)

    @property
    def rotation(self) -> Vector3:
        return Vector3(_Prism.GetRotation(self._entity_id)) * (180.0 / 3.141592653589793)

    @rotation.setter
    def rotation(self, value):
        _Prism.SetRotation(self._entity_id, value)

    @property
    def scale(self) -> Vector3:
        return Vector3(_Prism.GetScale(self._entity_id))

    @scale.setter
    def scale(self, value):
        _Prism.SetScale(self._entity_id, value)

    @property
    def forward(self) -> Vector3:
        from Prism.Math.Quaternion import Quaternion
        r = self.rotation
        q = Quaternion.Euler(r.x, r.y, r.z)
        return q * Vector3.Forward

    @property
    def right(self) -> Vector3:
        from Prism.Math.Quaternion import Quaternion
        r = self.rotation
        q = Quaternion.Euler(r.x, r.y, r.z)
        return q * Vector3.Right

    @property
    def up(self) -> Vector3:
        from Prism.Math.Quaternion import Quaternion
        r = self.rotation
        q = Quaternion.Euler(r.x, r.y, r.z)
        return q * Vector3.Up

    def SetPosition(self, x: float, y: float, z: float):
        _Prism.SetPosition(self._entity_id, Vector3(x, y, z))

    def SetRotation(self, x: float, y: float, z: float):
        _Prism.SetRotation(self._entity_id, Vector3(x, y, z))

    def SetScale(self, x: float, y: float, z: float):
        _Prism.SetScale(self._entity_id, Vector3(x, y, z))
