from Prism.Component import Component


class Behaviour(Component):
    def __init__(self):
        super().__init__()
        self._enabled = True

    @property
    def enabled(self) -> bool:
        return self._enabled

    @enabled.setter
    def enabled(self, value: bool):
        self._enabled = value

    @property
    def transform(self):
        return self.entity.transform if self.entity else None

    def GetComponent(self, type_name: str):
        return self.entity.GetComponent(type_name) if self.entity else None

    def HasComponent(self, type_name: str) -> bool:
        return self.entity.HasComponent(type_name) if self.entity else False

    def CreateComponent(self, type_name: str):
        if self.entity:
            return self.entity.create_component(type_name)
        return None

