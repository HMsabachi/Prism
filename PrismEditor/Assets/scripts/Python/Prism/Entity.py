import PrismNative as _Prism
from Prism.Core.Transform import Transform


class Entity:
    def __init__(self, entity_id: int = 0):
        self._id = entity_id
        self._transform = None

    @property
    def ID(self) -> int:
        return self._id

    @ID.setter
    def ID(self, value: int):
        self._id = value

    @property
    def Transform(self):
        if self._transform is None:
            self._transform = Transform(self._id)
        return self._transform

    def HasComponent(self, cls) -> bool:
        if issubclass(cls, Behaviour) and cls is not Behaviour:
            return _Prism.Prism_Entity_GetBehaviour(self._id, cls) is not None
        return _Prism.Prism_Entity_HasComponent(self._id, cls)

    def GetComponent(self, cls):
        if issubclass(cls, Behaviour) and cls is not Behaviour:
            return _Prism.Prism_Entity_GetBehaviour(self._id, cls)
        if self.HasComponent(cls):
            component = cls()
            component.Entity = self
            return component
        return None

    def CreateComponent(self, cls):
        if issubclass(cls, Behaviour) and cls is not Behaviour:
            return _Prism.Prism_Entity_AddBehaviour(self._id, cls)
        else:
            _Prism.Prism_Entity_CreateComponent(self._id, cls)
            component = cls()
            component.Entity = self
            return component
