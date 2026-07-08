import PrismNative as _Prism
from Prism.Math.Vector2 import Vector2
from Prism.Math.Vector3 import Vector3
from PrismEngine import Component


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
    """标记组件，Tag 通过 C++ ECS 直接管理。"""

    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  TransformComponent
# ════════════════════════════════════════════
from PrismEngine import TransformComponent


# ════════════════════════════════════════════
#  MeshRendererComponent
# ════════════════════════════════════════════

from PrismEngine import MeshRendererComponent


# ════════════════════════════════════════════
#  CameraComponent
# ════════════════════════════════════════════

class CameraComponent(Component):
    """Camera API 尚未实现。"""

    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  ScriptComponent
# ════════════════════════════════════════════

class ScriptComponent(Component):
    """Script API 尚未实现。"""

    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  SpriteRendererComponent
# ════════════════════════════════════════════

class SpriteRendererComponent(Component):
    """SpriteRenderer API 尚未实现。"""

    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  RigidBody2DComponent
# ════════════════════════════════════════════

class RigidBody2DComponent(Component):
    """C++ RigidBody2DComponent 的封装，提供 2D 物理控制。"""

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
    """2D 盒碰撞体标记组件。"""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  CircleCollider2DComponent
# ════════════════════════════════════════════

class CircleCollider2DComponent(Component):
    """2D 圆形碰撞体标记组件。"""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  RigidBodyComponent (3D)
# ════════════════════════════════════════════

class RigidBodyComponent(Component):
    """C++ RigidBodyComponent 的封装，提供 3D 物理控制。"""

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

    def Rotate(self, rotation: Vector3):
        _Prism.Prism_RigidBodyComponent_Rotate(self.Entity._id, rotation)

    @property
    def Mass(self) -> float:
        return _Prism.Prism_RigidBodyComponent_GetMass(self.Entity._id)

    @Mass.setter
    def Mass(self, value: float):
        _Prism.Prism_RigidBodyComponent_SetMass(self.Entity._id, value)

    @property
    def Layer(self) -> int:
        return _Prism.Prism_RigidBodyComponent_GetLayer(self.Entity._id)

    @property
    def BodyType(self) -> int:
        return _Prism.Prism_RigidBodyComponent_GetBodyType(self.Entity._id)

    @property
    def AngularVelocity(self):
        return Vector3(_Prism.Prism_RigidBodyComponent_GetAngularVelocity(self.Entity._id))

    @AngularVelocity.setter
    def AngularVelocity(self, value):
        _Prism.Prism_RigidBodyComponent_SetAngularVelocity(self.Entity._id, value)


# ════════════════════════════════════════════
#  BoxColliderComponent (3D)
# ════════════════════════════════════════════

class BoxColliderComponent(Component):
    """3D 盒碰撞体标记组件。"""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  SphereColliderComponent (3D)
# ════════════════════════════════════════════

class SphereColliderComponent(Component):
    """3D 球形碰撞体标记组件。"""
    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  CapsuleColliderComponent (3D)
# ════════════════════════════════════════════

class CapsuleColliderComponent(Component):
    """3D 胶囊碰撞体标记组件。"""
    def __init__(self):
        super().__init__()
