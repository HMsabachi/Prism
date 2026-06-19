from typing import Optional
import PrismNative as _Prism
from Prism.Math.Vector3 import Vector3
from Prism.Entity import Entity
from Prism.Physics.Collider import BoxCollider, SphereCollider, CapsuleCollider, MeshCollider, Collider


class RaycastHit:
    """Result of a physics raycast."""

    def __init__(self):
        self.EntityID: int = 0
        self.Position: Vector3 = Vector3.Zero
        self.Normal: Vector3 = Vector3.Zero
        self.Distance: float = 0.0


class _PhysicsGravity:
    def __get__(self, obj, objtype=None):
        return _Prism.Prism_Physics_GetGravity()
    def __set__(self, obj, value):
        _Prism.Prism_Physics_SetGravity(value)


class Physics:
    gravity = _PhysicsGravity()

    @staticmethod
    def Raycast(origin: Vector3, direction: Vector3, maxDistance: float, hit: Optional[RaycastHit] = None) -> bool:
        """Cast a ray into the physics scene. Returns True if something was hit."""
        result = _Prism.Prism_Physics_Raycast(origin, direction, maxDistance)
        if result is None:
            return False
        entityID, position, normal, distance = result
        if hit is not None:
            hit.EntityID = entityID
            hit.Position = position
            hit.Normal = normal
            hit.Distance = distance
        return True

    @staticmethod
    def OverlapBox(origin: Vector3, halfSize: Vector3) -> list[Collider]:
        """Returns all Colliders overlapping a box at origin with half-extents halfSize."""
        result = _Prism.Prism_Physics_OverlapBox(origin, halfSize)
        if result is None:
            return []
        return [Physics._HitDataToCollider(d) for d in result]

    @staticmethod
    def OverlapCapsule(origin: Vector3, radius: float, halfHeight: float) -> list[Collider]:
        """Returns all Colliders overlapping a capsule at origin with given radius and half-height."""
        result = _Prism.Prism_Physics_OverlapCapsule(origin, radius, halfHeight)
        if result is None:
            return []
        return [Physics._HitDataToCollider(d) for d in result]

    @staticmethod
    def OverlapSphere(origin: Vector3, radius: float) -> list[Collider]:
        """Returns all Colliders overlapping a sphere at origin with given radius."""
        result = _Prism.Prism_Physics_OverlapSphere(origin, radius)
        if result is None:
            return []
        return [Physics._HitDataToCollider(d) for d in result]

    @staticmethod
    def OverlapBoxNonAlloc(origin: Vector3, halfSize: Vector3, colliders: list) -> int:
        """Non-allocating overlap query. Only available in C#."""
        raise NotImplementedError("OverlapBoxNonAlloc is only available in C#")

    @staticmethod
    def OverlapCapsuleNonAlloc(origin: Vector3, radius: float, halfHeight: float, colliders: list) -> int:
        """Non-allocating overlap query. Only available in C#."""
        raise NotImplementedError("OverlapCapsuleNonAlloc is only available in C#")

    @staticmethod
    def OverlapSphereNonAlloc(origin: Vector3, radius: float, colliders: list) -> int:
        """Non-allocating overlap query. Only available in C#."""
        raise NotImplementedError("OverlapSphereNonAlloc is only available in C#")

    @staticmethod
    def _HitDataToCollider(data: tuple) -> Collider:
        """Convert native overlap hit tuple to a Collider object."""
        colliderType = data[1]
        entityID = data[0]
        isTrigger = data[2]
        if colliderType == 0:  # Box
            return BoxCollider(entityID, isTrigger,
                Vector3(data[3], data[4], data[5]),
                Vector3(data[6], data[7], data[8]))
        elif colliderType == 1:  # Sphere
            return SphereCollider(entityID, isTrigger, data[3])
        elif colliderType == 2:  # Capsule
            return CapsuleCollider(entityID, isTrigger, data[3], data[4])
        elif colliderType == 3:  # Mesh
            return MeshCollider(entityID, isTrigger, data[3])
        return None
