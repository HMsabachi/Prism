import PrismNative as _Prism
from Prism.Math.Vector2 import Vector2
from Prism.Math.Vector3 import Vector3
from Prism.Entity import Entity as Ent

class Component:
    """所有组件的基类，持有所属 Entity 的引用。"""
    Entity: "Ent" = None
    def __init__(self):
        pass


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
    """实体的 Tag 标签读写。"""

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

class TransformComponent(Component):
    def __init__(self):
        super().__init__()

    @property
    def Position(self):
        return Vector3(_Prism.Prism_TransformComponent_GetPosition(self.Entity._id))

    @Position.setter
    def Position(self, value):
        _Prism.Prism_TransformComponent_SetPosition(self.Entity._id, value)

    @property
    def Rotation(self):
        return Vector3(_Prism.Prism_TransformComponent_GetRotation(self.Entity._id)) * (180.0 / 3.141592653589793)

    @Rotation.setter
    def Rotation(self, value):
        _Prism.Prism_TransformComponent_SetRotation(self.Entity._id, value)

    @property
    def Scale(self):
        return Vector3(_Prism.Prism_TransformComponent_GetScale(self.Entity._id))

    @Scale.setter
    def Scale(self, value):
        _Prism.Prism_TransformComponent_SetScale(self.Entity._id, value)

    @property
    def LocalPosition(self):
        return Vector3(_Prism.Prism_TransformComponent_GetLocalPosition(self.Entity._id))

    @LocalPosition.setter
    def LocalPosition(self, value):
        _Prism.Prism_TransformComponent_SetLocalPosition(self.Entity._id, value)

    @property
    def LocalRotation(self):
        return Vector3(_Prism.Prism_TransformComponent_GetLocalRotation(self.Entity._id)) * (180.0 / 3.141592653589793)

    @LocalRotation.setter
    def LocalRotation(self, value):
        _Prism.Prism_TransformComponent_SetLocalRotation(self.Entity._id, value)

    @property
    def LocalScale(self):
        return Vector3(_Prism.Prism_TransformComponent_GetLocalScale(self.Entity._id))

    @LocalScale.setter
    def LocalScale(self, value):
        _Prism.Prism_TransformComponent_SetLocalScale(self.Entity._id, value)

    @property
    def Forward(self):
        result = _Prism.Prism_TransformComponent_GetTransform(self.Entity._id)
        return Vector3(result[5])

    @property
    def Right(self):
        result = _Prism.Prism_TransformComponent_GetTransform(self.Entity._id)
        return Vector3(result[4])

    @property
    def Up(self):
        result = _Prism.Prism_TransformComponent_GetTransform(self.Entity._id)
        return Vector3(result[3])

    @property
    def Transform(self):
        from Prism.Core.Transform import Transform as T
        result = _Prism.Prism_TransformComponent_GetTransform(self.Entity._id)
        return T(Vector3(result[0]), Vector3(result[1]), Vector3(result[2]),
                 Vector3(result[3]), Vector3(result[4]), Vector3(result[5]))

    @Transform.setter
    def Transform(self, value):
        _Prism.Prism_TransformComponent_SetTransform(
            self.Entity._id,
            value.Position, value.Rotation, value.Scale
        )

    @property
    def LocalTransform(self):
        from Prism.Core.Transform import Transform as T
        return T(self.LocalPosition, self.LocalRotation, self.LocalScale,
                 Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0))

    @LocalTransform.setter
    def LocalTransform(self, value):
        self.LocalPosition = value.Position
        self.LocalRotation = value.Rotation
        self.LocalScale = value.Scale

    def SetPosition(self, x, y, z):
        _Prism.Prism_TransformComponent_SetPosition(self.Entity._id, Vector3(x, y, z))

    def SetRotation(self, x, y, z):
        _Prism.Prism_TransformComponent_SetRotation(self.Entity._id, Vector3(x, y, z))

    def SetScale(self, x, y, z):
        _Prism.Prism_TransformComponent_SetScale(self.Entity._id, Vector3(x, y, z))


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
