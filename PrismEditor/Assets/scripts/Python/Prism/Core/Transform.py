import PrismNative as _Prism
from Prism.Math.Vector3 import Vector3


class Transform:
    """Wrapper for C++ TransformComponent, providing Position/Rotation/Scale access."""

    def __init__(self, entity_id: int):
        self._entity_id = entity_id

    @property
    def position(self) -> Vector3:
        x, y, z = _Prism.GetPosition(self._entity_id)
        return Vector3(x, y, z)

    @position.setter
    def position(self, value):
        _Prism.SetPosition(self._entity_id, (value.x, value.y, value.z))

    @property
    def rotation(self) -> Vector3:
        x, y, z = _Prism.GetRotation(self._entity_id)
        return Vector3(x, y, z) * (180.0 / 3.141592653589793)

    @rotation.setter
    def rotation(self, value):
        _Prism.SetRotation(self._entity_id, (value.x, value.y, value.z))

    @property
    def scale(self) -> Vector3:
        x, y, z = _Prism.GetScale(self._entity_id)
        return Vector3(x, y, z)

    @scale.setter
    def scale(self, value):
        _Prism.SetScale(self._entity_id, (value.x, value.y, value.z))

    def SetPosition(self, x: float, y: float, z: float):
        _Prism.SetPosition(self._entity_id, (x, y, z))

    def SetRotation(self, x: float, y: float, z: float):
        _Prism.SetRotation(self._entity_id, (x, y, z))

    def SetScale(self, x: float, y: float, z: float):
        _Prism.SetScale(self._entity_id, (x, y, z))
