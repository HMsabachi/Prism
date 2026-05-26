from Prism.Component import Component
from Prism.Entity import Entity as Ent


class Behaviour(Component):
    Entity: Ent = None  # 运行时由引擎设置，编辑器中为 None
    def __init__(self):
        super().__init__()
        self._enabled = True

    @property
    def Enabled(self) -> bool:
        return self._enabled

    @Enabled.setter
    def Enabled(self, value: bool):
        self._enabled = value

    @property
    def Transform(self):
        return self.Entity.Transform if self.Entity else None

    def GetComponent(self, cls):
        return self.Entity.GetComponent(cls) if self.Entity else None

    def HasComponent(self, cls) -> bool:
        return self.Entity.HasComponent(cls) if self.Entity else False

    def CreateComponent(self, cls):
        if self.Entity:
            return self.Entity.CreateComponent(cls)
        return None
