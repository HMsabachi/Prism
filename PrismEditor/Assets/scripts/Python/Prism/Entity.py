import PrismNative as _Prism
from Prism.Behaviour import Behaviour
from Prism.Core.Transform import Transform


class Entity:
    def __init__(self, entity_id: int = 0):
        self._id = entity_id
        self._transform = None

    @property
    def id(self) -> int:
        return self._id

    @id.setter
    def id(self, value: int):
        self._id = value

    @property
    def transform(self):
        if self._transform is None:
            self._transform = Transform(self._id)
        return self._transform

    def HasComponent(self, type_name: str) -> bool:
        return _Prism.HasComponent(self._id, type_name)

    def GetComponent(self, type_name: str):
        if self.HasComponent(type_name):
            return type_name
        return None

    def create_component(self, cls):
        if issubclass(cls, Behaviour) and cls is not Behaviour:
            return _Prism.AddBehaviour(self._id, cls.__module__, cls.__name__)
        else:
            type_name = f"{cls.__module__}.{cls.__name__}"
            _Prism.CreateComponent(self._id, type_name)
            component = cls()
            component.entity = self
            return component
