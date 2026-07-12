# Python 脚本引擎 — pybind11 架构文档

Prism Python 脚本系统基于 **pybind11** 实现，通过 `PrismEngine` 原生模块将 C++ 引擎能力暴露给 Python 脚本。Behaviour 类继承自 `Prism.Behaviour`，遵循与 C# 统一的生命周期模型。

---

## 1. 架构概览

```
Python 脚本文件 (.py)
    │
    ▼
CPython 3.13 嵌入式运行时
    │  pybind11::embed
    ▼
PrismEngine 原生模块（C++ 编译为 Python 扩展）
    ├─ Entity / Component / Behaviour  ← ECS 绑定
    ├─ Transform / MeshRenderer / RigidBody  ← 组件绑定
    ├─ Input / Time / Log / Noise  ← 引擎核心
    ├─ Mesh / Material / Texture2D  ← 资产绑定
    ├─ Physics / Collider / RaycastHit  ← 物理绑定
    └─ Math (Vector2/3/4, Quaternion, Matrix4) ← glm ↔ pyglm 桥接
    │
    ▼
PythonScriptEngine（C++ 侧管理）
    ├─ 解释器生命周期（Initialize / Shutdown）
    ├─ 脚本实例化与存储（s_PythonScriptObjects）
    ├─ Behaviour 挂载/移除（AddBehaviour / RemoveBehaviour）
    └─ 脚本热重载（ReloadPythonScripts）
```

### 与旧版（CPython C API 封装层）的根本区别

| | 旧版（PythonScriptCore） | 新版（pybind11） |
|---|---|---|
| 互操作层 | 手写 CPython C API (`ScriptRef`/`ScriptObject`) | pybind11 自动绑定 |
| 类型系统 | `__annotations__` 类型注解 | pybind11 `PYBIND11_MODULE` 强类型 |
| C++→Python | `NativeModule::AddFunction` | `py::class_<>::def()` |
| 数学类型 | 无桥接 | glm ↔ pyglm Buffer Protocol |
| 资产字段 | 不支持 | `PythonMesh`/`PythonMaterial`/`PythonTexture2D` 类型化字段 |
| 文件 | `PythonScriptCore.h/.cpp`（已删除） | `PythonScriptEngine` + `PythonScriptWrappers` 等 |

---

## 2. 文件结构

```
Prism/src/Scripting/Python/
├── PythonScriptEngine.h/.cpp       # 解释器生命周期、实例管理
├── PythonScriptEngineRegistry.h/.cpp  # Component 类型注册
├── PythonScriptMetaRegistry.h/.cpp    # Python 类元数据缓存
├── PythonScriptStorage.h/.cpp         # 脚本实例存储
├── PythonScriptWrappers.h/.cpp        # PrismEngine 模块定义（PYBIND11_MODULE）
├── PythonField.h/.inl                 # 公共字段序列化支持
└── PythonScriptTypeCasters.h          # glm ↔ pyglm 类型转换
```

---

## 3. 初始化与生命周期

```cpp
// 引擎启动时
PythonScriptEngine::Initialize();
// → PyImport_AppendInittab("PrismEngine", PyInit_PrismEngine)
// → py::initialize_interpreter()
// → sys.path 添加 Assets/scripts/Python
// → 注册所有 Component 类型
// → 构建 PythonScriptMetaRegistry 缓存

// 引擎关闭时
PythonScriptEngine::Shutdown();
// → 释放所有脚本对象
// → py::finalize_interpreter()
```

---

## 4. PrismEngine 模块 API

`PrismEngine` 是 C++ 通过 `PYBIND11_MODULE(PrismEngine, m)` 定义的 Python 原生模块，在 `PythonScriptWrappers.cpp` 中注册。Python 侧通过 `from PrismEngine import *` 或模块子包（`Prism`、`Prism.Math`、`Prism.Component`）使用。

### 4.1 Entity

```python
entity = Entity(entityID)
entity.ID          # uint64
entity.Transform   # TransformComponent

# 组件操作
entity.GetComponent(Prism.Component.CameraComponent)
entity.HasComponent(Prism.Component.RigidBodyComponent)
entity.CreateComponent(Prism.Component.BoxColliderComponent)

# 查找
Entity.FindEntityByTag("Player")
Entity.FindEntityByID(12345)
```

### 4.2 Behaviour（脚本基类）

```python
class MyScript(Prism.Behaviour):
    def Awake(self): ...
    def OnCreate(self): ...
    def OnEnable(self): ...
    def OnUpdate(self): ...
    def LateUpdate(self): ...
    def OnFixedUpdate(self): ...
    def OnDisable(self): ...
    def OnDestroy(self): ...

    # 碰撞回调
    def OnCollisionBegin(self, collider): ...
    def OnCollisionEnd(self, collider): ...
    def OnCollision2DBegin(self, collider): ...
    def OnCollision2DEnd(self, collider): ...

    # 可用属性/方法
    self.ID          # Behaviour UUID
    self.Enabled     # 启用/禁用
    self.Entity      # 所属 Entity
    self.Transform   # 快捷 TransformComponent
    self.GetComponent(cls)
    self.HasComponent(cls)
    self.CreateComponent(cls)
```

### 4.3 Component 类型一览

| Python 类型 | 对应 C++ Component | 说明 |
|---|---|---|
| `TransformComponent` | `TransformComponent` | 位置/旋转/缩放 + 方向向量 |
| `MeshRendererComponent` | `MeshRendererComponent` | 网格 + 子网格材质列表 |
| `CameraComponent` | `CameraComponent` | 相机 |
| `TagComponent` | `TagComponent` | 标签名 |
| `RigidBodyComponent` | `RigidBodyComponent` | 3D 刚体（PhysX） |
| `BoxColliderComponent` | `BoxColliderComponent` | 3D 盒碰撞体 |
| `SphereColliderComponent` | `SphereColliderComponent` | 3D 球碰撞体 |
| `CapsuleColliderComponent` | `CapsuleColliderComponent` | 3D 胶囊碰撞体 |
| `RigidBody2DComponent` | `RigidBody2DComponent` | 2D 刚体（Box2D） |
| `BoxCollider2DComponent` | `BoxCollider2DComponent` | 2D 盒碰撞体 |
| `CircleCollider2DComponent` | `CircleCollider2DComponent` | 2D 圆碰撞体 |
| `SpriteRendererComponent` | `SpriteRendererComponent` | 2D 精灵 |
| `ScriptComponent` | `ScriptComponent` | 脚本容器 |

### 4.4 资产类型

```python
# Mesh
mesh = Mesh("models/teapot.obj")       # 从文件加载
mesh = Mesh(handle)                     # 从 AssetHandle 包装

# Material
mat = Material("Standard/PrismPBR")     # 从 Shader 名创建
mat.SetFloat("_Metallic", 0.8)
mat.SetColor("_Color", Vector4(1,0,0,1))
mat.SetTexture("_MainTex", texture)
mat.SetKeyword("ALBEDO_MAP", True)

# Texture2D
tex = Texture2D(512, 512)              # 创建空纹理
tex = Texture2D(handle)                # 从 AssetHandle 包装
```

### 4.5 引擎核心

```python
Time.DeltaTime       # float (只读)
Time.TimeScale       # float (读写)
Time.FixedDeltaTime  # float (读写)
Time.FrameCount      # int (只读)

Input.IsKeyPressed(KeyCode.W)
Input.GetMousePosition()        # Vector2
Input.IsMouseButtonPressed(MouseButton.Left)

Log.Info("message")
Log.Warn("message")
Log.Error("message")
```

### 4.6 物理

```python
Physics.Gravity = -9.81

# 射线检测
hit = RaycastHit()
if Physics.Raycast(origin, direction, maxDistance, hit):
    print(hit.Position, hit.Normal, hit.EntityID)

# 重叠检测
hits = Physics.OverlapBox(origin, halfSize)
for collider in hits:
    print(collider.Entity, collider.IsTrigger)

Physics.OverlapSphere(origin, radius)
Physics.OverlapCapsule(origin, radius, halfHeight)
```

---

## 5. 类型转换（glm ↔ pyglm）

`PythonScriptTypeCasters.h` 通过 pybind11 类型 caster 实现 glm 和 pyglm 之间的双向自动转换：

| C++ 类型 | Python 类型 | 模块 |
|---|---|---|
| `glm::vec2` | `Vector2` | `Prism.Math` |
| `glm::vec3` | `Vector3` | `Prism.Math` |
| `glm::vec4` | `Vector4` | `Prism.Math` |
| `glm::mat4` | `Matrix4` | `Prism.Math` |
| `glm::quat` | `Quaternion` | `Prism.Math` |

转换在 `py::class_` 绑定中自动执行，Behaviour 方法参数和返回值无需手动处理。

---

## 6. 公共字段（编辑器暴露）

Python Behaviour 类中带类型注解的字段会自动暴露到编辑器属性面板：

```python
class MyScript(Prism.Behaviour):
    speed: float = 10.0          # 显示为 Float 输入框
    target: Vector3              # 显示为 Vector3 输入框
    mesh: Mesh                   # 显示为 Mesh 资产拖放
    material: Material           # 显示为 Material 资产拖放
```

支持的字段类型：`float`, `int`, `bool`, `str`, `Vector2`, `Vector3`, `Vector4`, `Mesh`, `Material`, `Texture2D`。

`PythonField.h/.inl` 负责序列化/反序列化字段值，`PythonScriptMetaRegistry` 在脚本加载时扫描 `__annotations__` 并缓存字段元数据。

---

## 7. 脚本热重载

```cpp
// 触发重载
PythonScriptEngine::ReloadPythonScripts();

// 重载前后回调
auto preToken = PythonScriptEngine::RegisterPreUnloadCallback([]() {
    // 保存脚本状态...
});
auto postToken = PythonScriptEngine::RegisterPostReloadCallback([]() {
    // 恢复脚本状态...
});
```

流程：触发 PreUnload 回调 → 释放所有脚本对象 → 清空元数据缓存 → 重建缓存 → 触发 PostReload 回调。编辑器 Play 模式下可一键触发。

---

## 8. 扩展指南

### 添加新的 Python 组件绑定

1. 在 `PythonScriptWrappers.cpp` 中定义包装类：
```cpp
class PythonMyComponent : public PythonComponent
{
public:
    int GetValue() const { return GetEntityImpt().GetComponent<MyComponent>().Value; }
    void SetValue(int v) { GetEntityImpt().GetComponent<MyComponent>().Value = v; }
};
```

2. 在 `PYBIND11_MODULE(PrismEngine, m)` 中注册：
```cpp
py::class_<PythonMyComponent, PythonComponent>(m, "MyComponent")
    .def(py::init<>())
    .def_property("Value", &PythonMyComponent::GetValue, &PythonMyComponent::SetValue);
```

3. 在 `PythonScriptEngineRegistry.cpp` 中注册到 Component 类型系统：
```cpp
RegisterPythonComponent<MyComponent>(compMod);
```

### 添加新的资产类型

1. 继承 `PythonAsset`，实现包装类
2. 在 `PYBIND11_MODULE` 中注册
3. 在 `RegisterAllPythonTypes()` 中添加类型缓存条目
