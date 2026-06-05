from typing import Optional
import PrismNative as _Prism
from Prism.Math.Vector3 import Vector3
from Prism.Entity import Entity


class RaycastHit:
    """Result of a physics raycast."""

    def __init__(self):
        self.EntityID: int = 0
        self.Position: Vector3 = Vector3.Zero
        self.Normal: Vector3 = Vector3.Zero
        self.Distance: float = 0.0


class Physics:
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
