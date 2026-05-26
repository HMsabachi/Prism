import PrismNative as _Prism
from Prism.Math.Vector3 import Vector3


class Transform:
    """Wrapper for C++ TransformComponent, providing Position/Rotation/Scale access."""

    def __init__(self, entity_id: int):
        self._entity_id = entity_id

    @property
    def Position(self) -> Vector3:
        return Vector3(_Prism.Prism_TransformComponent_GetPosition(self._entity_id))

    @Position.setter
    def Position(self, value):
        _Prism.Prism_TransformComponent_SetPosition(self._entity_id, value)

    @property
    def Rotation(self) -> Vector3:
        return Vector3(_Prism.Prism_TransformComponent_GetRotation(self._entity_id)) * (180.0 / 3.141592653589793)

    @Rotation.setter
    def Rotation(self, value):
        _Prism.Prism_TransformComponent_SetRotation(self._entity_id, value)

    @property
    def Scale(self) -> Vector3:
        return Vector3(_Prism.Prism_TransformComponent_GetScale(self._entity_id))

    @Scale.setter
    def Scale(self, value):
        _Prism.Prism_TransformComponent_SetScale(self._entity_id, value)

    @property
    def Forward(self) -> Vector3:
        from Prism.Math.Quaternion import Quaternion
        r = self.Rotation
        q = Quaternion.Euler(r.x, r.y, r.z)
        return q * Vector3.Forward

    @property
    def Right(self) -> Vector3:
        from Prism.Math.Quaternion import Quaternion
        r = self.Rotation
        q = Quaternion.Euler(r.x, r.y, r.z)
        return q * Vector3.Right

    @property
    def Up(self) -> Vector3:
        from Prism.Math.Quaternion import Quaternion
        r = self.Rotation
        q = Quaternion.Euler(r.x, r.y, r.z)
        return q * Vector3.Up

    def SetPosition(self, x: float, y: float, z: float):
        _Prism.Prism_TransformComponent_SetPosition(self._entity_id, Vector3(x, y, z))

    def SetRotation(self, x: float, y: float, z: float):
        _Prism.Prism_TransformComponent_SetRotation(self._entity_id, Vector3(x, y, z))

    def SetScale(self, x: float, y: float, z: float):
        _Prism.Prism_TransformComponent_SetScale(self._entity_id, Vector3(x, y, z))
