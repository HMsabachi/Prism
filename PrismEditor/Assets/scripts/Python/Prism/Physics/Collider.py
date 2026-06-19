from Prism.Component import Component
from Prism.Entity import Entity as Ent
from Prism.Math.Vector3 import Vector3


class Collider:
    """所有 Collider 的基类，对应 C# Prism.Collider。"""

    EntityID: int = 0
    IsTrigger: bool = False


class BoxCollider(Collider):
    """盒碰撞体。"""

    Size: Vector3 = Vector3.One
    Offset: Vector3 = Vector3.Zero

    def __init__(self, entityID: int, isTrigger: bool, size: Vector3, offset: Vector3):
        self.EntityID = entityID
        self.IsTrigger = isTrigger
        self.Size = size
        self.Offset = offset


class SphereCollider(Collider):
    """球形碰撞体。"""

    Radius: float = 0.5

    def __init__(self, entityID: int, isTrigger: bool, radius: float):
        self.EntityID = entityID
        self.IsTrigger = isTrigger
        self.Radius = radius


class CapsuleCollider(Collider):
    """胶囊碰撞体。"""

    Radius: float = 0.5
    Height: float = 1.0

    def __init__(self, entityID: int, isTrigger: bool, radius: float, height: float):
        self.EntityID = entityID
        self.IsTrigger = isTrigger
        self.Radius = radius
        self.Height = height


class MeshCollider(Collider):
    """网格碰撞体。"""

    Mesh = None

    def __init__(self, entityID: int, isTrigger: bool, meshHandle: int):
        from Prism.Renderer.Mesh import Mesh as MeshClass
        self.EntityID = entityID
        self.IsTrigger = isTrigger
        self.Mesh = MeshClass(meshHandle)
