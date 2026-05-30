from Prism.Component import Component
from Prism.Entity import Entity as Ent
from Prism.Math.Vector3 import Vector3


class Collider:
    """所有 Collider 的基类，对应 C# Prism.Collider。"""

    class ColliderType:
        Box = 0
        Sphere = 1
        Capsule = 2
        Mesh = 3

    ColliderEntity: Ent = None
    Type: int = ColliderType.Box
    IsTrigger: bool = False


class BoxCollider(Collider):
    """盒碰撞体。"""

    Size: Vector3 = Vector3.One()
    Offset: Vector3 = Vector3.Zero()

    def __init__(self, entityID: int, size: Vector3, offset: Vector3, isTrigger: bool):
        self.ColliderEntity = Ent(entityID)
        self.Type = Collider.ColliderType.Box
        self.Size = size
        self.Offset = offset
        self.IsTrigger = isTrigger


class SphereCollider(Collider):
    """球形碰撞体。"""

    Radius: float = 0.5

    def __init__(self, entityID: int, radius: float, isTrigger: bool):
        self.ColliderEntity = Ent(entityID)
        self.Type = Collider.ColliderType.Sphere
        self.Radius = radius
        self.IsTrigger = isTrigger


class CapsuleCollider(Collider):
    """胶囊碰撞体。"""

    Radius: float = 0.5
    Height: float = 1.0

    def __init__(self, entityID: int, radius: float, height: float, isTrigger: bool):
        self.ColliderEntity = Ent(entityID)
        self.Type = Collider.ColliderType.Capsule
        self.Radius = radius
        self.Height = height
        self.IsTrigger = isTrigger


class MeshCollider(Collider):
    """网格碰撞体。"""

    Mesh = None

    def __init__(self, entityID: int, mesh, isTrigger: bool):
        self.ColliderEntity = Ent(entityID)
        self.Type = Collider.ColliderType.Mesh
        self.Mesh = mesh
        self.IsTrigger = isTrigger
