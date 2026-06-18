import PrismNative as _Prism
from Prism.Component import Component, TagComponent, TransformComponent, MeshRendererComponent, CameraComponent, ScriptComponent, SpriteRendererComponent, RigidBody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent, RigidBodyComponent, BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, ForceMode
from Prism.Entity import Entity as Ent


class Behaviour(Component):
    Entity: Ent = None
    ID: int = 0

    @property
    def Enabled(self) -> bool:
        if self.ID == 0:
            return True
        return _Prism.Prism_Behaviour_GetEnabled(self.ID)

    @Enabled.setter
    def Enabled(self, value: bool) -> None:
        if self.ID == 0:
            return
        _Prism.Prism_Behaviour_SetEnabled(self.ID, value)

    @property
    def Transform(self) -> TransformComponent:
        return self.Entity.Transform if self.Entity else None

    def GetComponent(self, cls) -> Component:
        return self.Entity.GetComponent(cls) if self.Entity else None

    def HasComponent(self, cls) -> bool:
        return self.Entity.HasComponent(cls) if self.Entity else False

    def CreateComponent(self, cls) -> Component:
        if self.Entity:
            return self.Entity.CreateComponent(cls)
        return None
