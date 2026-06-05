from Prism.Component import Component
from Prism.Entity import Entity as Ent
from Prism.Math.Vector3 import Vector3


class ColliderType:
    Box = 0
    Sphere = 1
    Capsule = 2
    Mesh = 3


class Collider:
    """所有 Collider 的基类，对应 C# Prism.Collider。"""

    Type: int = ColliderType.Box
    EntityID: int = 0
    IsTrigger: bool = False

    def IsBox(self) -> bool:
        return self.Type == ColliderType.Box

    def IsSphere(self) -> bool:
        return self.Type == ColliderType.Sphere

    def IsCapsule(self) -> bool:
        return self.Type == ColliderType.Capsule


class BoxCollider(Collider):
    """盒碰撞体。"""

    Size: Vector3 = Vector3.One
    Offset: Vector3 = Vector3.Zero

    def __init__(self, entityID: int, isTrigger: bool, size: Vector3, offset: Vector3):
        self.Type = ColliderType.Box
        self.EntityID = entityID
        self.IsTrigger = isTrigger
        self.Size = size
        self.Offset = offset


class SphereCollider(Collider):
    """球形碰撞体。"""

    Radius: float = 0.5

    def __init__(self, entityID: int, isTrigger: bool, radius: float):
        self.Type = ColliderType.Sphere
        self.EntityID = entityID
        self.IsTrigger = isTrigger
        self.Radius = radius


class CapsuleCollider(Collider):
    """胶囊碰撞体。"""

    Radius: float = 0.5
    Height: float = 1.0

    def __init__(self, entityID: int, isTrigger: bool, radius: float, height: float):
        self.Type = ColliderType.Capsule
        self.EntityID = entityID
        self.IsTrigger = isTrigger
        self.Radius = radius
        self.Height = height


class MeshCollider(Collider):
    """网格碰撞体。"""

    Mesh = None

    def __init__(self, entityID: int, isTrigger: bool, filepath: str):
        from Prism.Renderer.Mesh import Mesh as MeshClass
        self.EntityID = entityID
        self.IsTrigger = isTrigger
        self.Mesh = MeshClass(filepath)
