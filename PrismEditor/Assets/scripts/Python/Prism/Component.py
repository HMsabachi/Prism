import PrismNative as _Prism
from Prism.Math.Vector2 import Vector2
from Prism.Math.Vector3 import Vector3


class Component:
    """Base class for all components. Holds a reference to the owning Entity."""

    def __init__(self):
        self.Entity = None


# ════════════════════════════════════════════
#  ForceMode
# ════════════════════════════════════════════

class ForceMode:
    Force = 0
    Impulse = 1
    VelocityChange = 2
    Acceleration = 3


# ════════════════════════════════════════════
#  TagComponent
# ════════════════════════════════════════════

class TagComponent(Component):
    """Provides tag access for the entity."""

    def __init__(self):
        super().__init__()

    @property
    def Tag(self) -> str:
        return _Prism.Prism_TagComponent_GetTag(self.Entity._id)

    @Tag.setter
    def Tag(self, value: str):
        _Prism.Prism_TagComponent_SetTag(self.Entity._id, value)


# ════════════════════════════════════════════
#  TransformComponent
# ════════════════════════════════════════════

# NOTE: TransformComponent is wrapped by Prism.Core.Transform.
# Use `entity.Transform` to access Position/Rotation/Scale.
# This class exists for HasComponent/CreateComponent TypeId registration.


class TransformComponent(Component):
    """Minimal marker for C++ TransformComponent TypeId registration."""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  MeshComponent
# ════════════════════════════════════════════

class MeshComponent(Component):
    """TODO: Requires Mesh / Material / MaterialInstance C++ bridge.

    C# API:
        Mesh Mesh { get; set; }
        MaterialInstance GetMaterial(int index)
        void SetMaterial(int index, MaterialInstance material)
        int GetMaterialCount()
        void SetOverrideMaterial(MaterialInstance material)
        MaterialInstance GetOverrideMaterial()
    """

    def __init__(self):
        super().__init__()
        # TODO: Mesh bridge not yet implemented for Python


# ════════════════════════════════════════════
#  CameraComponent
# ════════════════════════════════════════════

class CameraComponent(Component):
    """TODO: Camera API not yet implemented."""

    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  MaterialComponent
# ════════════════════════════════════════════

class MaterialComponent(Component):
    """TODO: Requires MaterialInstance C++ bridge.

    C# API:
        MaterialInstance Material { get; set; }
    """

    def __init__(self):
        super().__init__()
        # TODO: Material bridge not yet implemented for Python


# ════════════════════════════════════════════
#  ScriptComponent
# ════════════════════════════════════════════

class ScriptComponent(Component):
    """TODO: Script API not yet implemented."""

    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  SpriteRendererComponent
# ════════════════════════════════════════════

class SpriteRendererComponent(Component):
    """TODO: SpriteRenderer API not yet implemented."""

    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  RigidBody2DComponent
# ════════════════════════════════════════════

class RigidBody2DComponent(Component):
    """Wrapper for C++ RigidBody2DComponent, providing 2D physics controls."""

    def __init__(self):
        super().__init__()

    def ApplyLinearImpulse(self, impulse: Vector2, offset: Vector2, wake: bool = True):
        _Prism.Prism_RigidBody2DComponent_ApplyLinearImpulse(self.Entity._id, impulse, offset, wake)

    @property
    def LinearVelocity(self) -> Vector2:
        return Vector2(_Prism.Prism_RigidBody2DComponent_GetLinearVelocity(self.Entity._id))

    @LinearVelocity.setter
    def LinearVelocity(self, value):
        _Prism.Prism_RigidBody2DComponent_SetLinearVelocity(self.Entity._id, value)

    def GetLinearVelocity(self) -> Vector2:
        return Vector2(_Prism.Prism_RigidBody2DComponent_GetLinearVelocity(self.Entity._id))

    def SetLinearVelocity(self, velocity: Vector2):
        _Prism.Prism_RigidBody2DComponent_SetLinearVelocity(self.Entity._id, velocity)


# ════════════════════════════════════════════
#  BoxCollider2DComponent
# ════════════════════════════════════════════

class BoxCollider2DComponent(Component):
    """Marker component for 2D box collider."""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  CircleCollider2DComponent
# ════════════════════════════════════════════

class CircleCollider2DComponent(Component):
    """Marker component for 2D circle collider."""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  RigidBodyComponent (3D)
# ════════════════════════════════════════════

class RigidBodyComponent(Component):
    """Wrapper for C++ RigidBodyComponent, providing 3D physics controls."""

    def __init__(self):
        super().__init__()

    def AddForce(self, force: Vector3, forceMode: int = ForceMode.Force):
        _Prism.Prism_RigidBodyComponent_AddForce(self.Entity._id, force, forceMode)

    def AddTorque(self, torque: Vector3, forceMode: int = ForceMode.Force):
        _Prism.Prism_RigidBodyComponent_AddTorque(self.Entity._id, torque, forceMode)

    @property
    def LinearVelocity(self) -> Vector3:
        return Vector3(_Prism.Prism_RigidBodyComponent_GetLinearVelocity(self.Entity._id))

    @LinearVelocity.setter
    def LinearVelocity(self, value):
        _Prism.Prism_RigidBodyComponent_SetLinearVelocity(self.Entity._id, value)

    def GetLinearVelocity(self) -> Vector3:
        return Vector3(_Prism.Prism_RigidBodyComponent_GetLinearVelocity(self.Entity._id))

    def SetLinearVelocity(self, velocity: Vector3):
        _Prism.Prism_RigidBodyComponent_SetLinearVelocity(self.Entity._id, velocity)


# ════════════════════════════════════════════
#  BoxColliderComponent (3D)
# ════════════════════════════════════════════

class BoxColliderComponent(Component):
    """Marker component for 3D box collider."""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  SphereColliderComponent (3D)
# ════════════════════════════════════════════

class SphereColliderComponent(Component):
    """Marker component for 3D sphere collider."""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  CapsuleColliderComponent (3D)
# ════════════════════════════════════════════

class CapsuleColliderComponent(Component):
    """Marker component for 3D capsule collider."""
    def __init__(self):
        super().__init__()
