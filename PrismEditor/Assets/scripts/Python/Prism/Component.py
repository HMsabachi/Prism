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

class MeshRendererComponent(Component):
    def __init__(self):
        super().__init__()

    @property
    def Mesh(self):
        from Prism.Renderer.Mesh import Mesh
        handle = _Prism.Prism_MeshRendererComponent_GetMesh(self.Entity._id)
        return Mesh(handle) if handle else None

    @Mesh.setter
    def Mesh(self, value):
        h = value._handle if value else 0
        _Prism.Prism_MeshRendererComponent_SetMesh(self.Entity._id, h)

    @property
    def Material(self):
        from Prism.Renderer.Material import Material
        handle = _Prism.Prism_MeshRendererComponent_GetMaterial(self.Entity._id, 0)
        return Material(handle) if handle else None

    @Material.setter
    def Material(self, value):
        h = value._handle if value else 0
        _Prism.Prism_MeshRendererComponent_SetMaterial(self.Entity._id, 0, h)

    @property
    def Materials(self):
        from Prism.Renderer.Material import Material
        handles = _Prism.Prism_MeshRendererComponent_GetMaterials(self.Entity._id)
        return [Material(h) if h else None for h in handles]

    @Materials.setter
    def Materials(self, value):
        handles = tuple(m._handle if m and m._handle else 0 for m in value)
        _Prism.Prism_MeshRendererComponent_SetMaterials(self.Entity._id, handles)

    def GetMaterial(self, index):
        from Prism.Renderer.Material import Material
        handle = _Prism.Prism_MeshRendererComponent_GetMaterial(self.Entity._id, index)
        return Material(handle) if handle else None

    def SetMaterial(self, index, material):
        h = material._handle if material else 0
        _Prism.Prism_MeshRendererComponent_SetMaterial(self.Entity._id, index, h)

    def GetMaterialCount(self):
        return _Prism.Prism_MeshRendererComponent_GetMaterialCount(self.Entity._id)


# ════════════════════════════════════════════
#  CameraComponent
# ════════════════════════════════════════════

class CameraComponent(Component):
    """Camera API 尚未实现。"""

    def __init__(self):
        super().__init__()


# ════════════════════════════════════════════
#  MaterialComponent
# ════════════════════════════════════════════

# MaterialComponent removed — use MeshRendererComponent.GetMaterial/SetMaterial instead


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
