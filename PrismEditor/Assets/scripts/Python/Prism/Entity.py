from typing import Optional
import PrismNative as _Prism
from Prism.Core.Transform import Transform


class Entity:
    _id: int
    _transform: Optional[Transform]
    def __init__(self, entity_id: int = 0):
        self._id: int = entity_id
        self._transform: Optional[Transform] = None

    @property
    def ID(self) -> int:
        return self._id

    @ID.setter
    def ID(self, value: int) -> None:
        self._id = value

    @property
    def Transform(self) -> "Transform":
        if self._transform is None:
            self._transform = Transform(self._id)
        return self._transform

    from Prism.Math.Matrix4 import Matrix4
    def GetTransform(self) -> "Matrix4":
        return Matrix4(_Prism.Prism_Entity_GetTransform(self._id))

    def SetTransform(self, transform: "Matrix4") -> None:
        _Prism.Prism_Entity_SetTransform(self._id, transform)

    def HasComponent(self, cls) -> bool:
        from Prism.Behaviour import Behaviour
        if issubclass(cls, Behaviour) and cls is not Behaviour:
            return _Prism.Prism_Entity_GetBehaviour(self._id, cls) is not None
        return _Prism.Prism_Entity_HasComponent(self._id, cls)

    def GetComponent(self, cls):
        from Prism.Behaviour import Behaviour
        if issubclass(cls, Behaviour) and cls is not Behaviour:
            return _Prism.Prism_Entity_GetBehaviour(self._id, cls)
        if self.HasComponent(cls):
            component = cls()
            component.Entity = self
            return component
        return None

    def CreateComponent(self, cls):
        from Prism.Behaviour import Behaviour
        if issubclass(cls, Behaviour) and cls is not Behaviour:
            return _Prism.Prism_Entity_AddBehaviour(self._id, cls)
        else:
            _Prism.Prism_Entity_CreateComponent(self._id, cls)
            component = cls()
            component.Entity = self
            return component

    @staticmethod
    def FindEntityByTag(tag: str) -> Optional["Entity"]:
        eid = _Prism.Prism_Entity_FindEntityByTag(tag)
        if eid:
            return Entity(eid)
        return None
