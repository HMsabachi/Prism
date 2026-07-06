from typing import Optional
import PrismNative as _Prism
from Prism.Core import Log


class Entity:
    _id: int
    def __init__(self, entity_id: int = 0):
        self._id: int = entity_id

    def __del__(self):
        Log.Trace("Destroyed Entity {}", self._id)

    @property
    def ID(self) -> int:
        return self._id

    @ID.setter
    def ID(self, value: int) -> None:
        self._id = value
        Log.Trace("Created Entity {}", self._id)

    @property
    def Transform(self):
        from Prism.Component import TransformComponent
        return self.GetComponent(TransformComponent)

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

    @staticmethod
    def FindEntityByID(entity_id: int) -> Optional["Entity"]:
        # TODO: Verify the entity id
        return Entity(entity_id)
