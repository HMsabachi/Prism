# Prism Engine

![Prism Engine Logo](https://github.com/HMsabachi/Prism/blob/main/Prism/assets/logo/WhiteLogo.png?raw=true "Prism Engine")

**Prism** 是一个轻量级、模块化、用 **C++17** 开发的跨平台游戏引擎。渲染后端使用 **OpenGL**（现代 API，如 `glCreateBuffer`、`glCreateVertexArrays` 等），**多语言脚本系统**支持 **C#**（.NET 9 + Rolky）和 **Python 3.13**（CPython 嵌入式），编辑器使用 **ImGui** 构建。

脚本系统采用 **Entity-Behaviour 架构**（类似 Unity），每个 Entity 是一个轻量容器，可挂载多个 Behaviour 组件。C# 和 Python 脚本遵循相同的生命周期模型（`Awake` → `OnCreate` → `OnEnable` → `OnUpdate` → `LateUpdate` → `OnFixedUpdate` → `OnDisable` → `OnDestroy`），并支持 2D/3D 碰撞回调。

> **注意：最新开发进度请切换到 `Prism3D` 分支**（`git checkout Prism3D`），所有最新提交均在此分支。

目前已实现现代渲染管线（PBR + HDR + MSAA + Compute Shader）、Scene 系统、基于 entt 的 ECS 架构、C#/Python 双语言脚本系统、Entity-Behaviour 组件化开发模式、ImGui 编辑器框架等功能。目标是打造一个清晰、易扩展、高性能的引擎，适合学习和中小型游戏开发。

## 快速开始

### 1. 克隆仓库并切换分支

```bash
git clone --recursive https://github.com/HMsabachi/Prism.git
cd Prism
git checkout Prism3D
```

### 2. 生成项目文件

- **Windows**：双击运行 `Win-GenerateProjects.bat`
- **其他平台**：手动执行

```bash
premake5 vs2022        # Windows Visual Studio
premake5 gmake         # Linux / macOS Makefile
premake5 xcode4        # macOS Xcode
```

### 3. 编译并运行

编译 `PrismEditor` 项目并运行，即可看到完整的编辑器界面，包含场景层级面板、属性面板、ImGuizmo 变换工具和视口渲染。

## 截图 / 演示

![Prism Engine UI](docs/Screenshot/Editor2.png "Prism Engine UI")

![Prism Engine UI](docs/Screenshot/Editor3.png "Prism Engine UI")

## 特性一览

### C# 脚本系统（基于 Entity-Behaviour 架构）
- **多个 Behaviour 组件**：同一 Entity 可挂载零至多个 Behaviour 子类，每个 Behaviour 是独立的脚本实例
- C++/C# 互操作与反射（基于 Rolky）
- .NET 9 运行时集成
- Behaviour 生命周期：`Awake` → `OnCreate` → `OnEnable` → `OnUpdate` → `LateUpdate` → `OnFixedUpdate` → `OnDisable` → `OnDestroy`
- InternalCall 自动绑定机制（C# 端使用 `delegate* unmanaged[Cdecl]<>` 函数指针）
- 编辑器运行时脚本热重载（支持 Play 时自动重载）
- C# 端完整 API：Input、Time、Log、Math（Vector2/3/4、Quaternion、Matrix4）、Renderer（Material、Mesh、Texture2D）
- C# 材质 API：`MaterialInstance.Set/GetMaterial`、`SetOverrideMaterial`、`SetKeyword/IsKeywordEnabled`
- C# 端物理 API：`RigidBody2DComponent.ApplyLinearImpulse`、`RigidBodyComponent.AddForce/AddTorque`（2D/3D 物理）、碰撞回调（`AddCollisionBeginCallback` / `AddCollisionEndCallback`）
- C# Entity API：`FindEntityByTag`、`HasComponent<T>`、`GetComponent<T>`、`CreateComponent<T>`
- 示例脚本：MapGenerator（程序化地形）、PlayerCube（2D 物理控制）、PlayerCube3D / PlayerSphere（3D 物理控制）、BasicController（相机移动）、Sink（下沉平台）、RandomColor（随机颜色）

### Python 脚本系统（Entity-Behaviour 架构）
- **Python 3.13** 嵌入式 CPython 运行时，Behaviour 子类自动注册为可挂载脚本组件
- **多 Behaviour 支持**：同一 Entity 可挂载任意数量 Python Behaviour，支持多脚本混合（C# + Python 共存）
- **完整生命周期**：`Awake` → `OnCreate` → `OnEnable` → `OnUpdate` → `LateUpdate` → `OnFixedUpdate` → `OnDisable` → `OnDestroy`
- **碰撞回调**：`OnCollisionBegin` / `OnCollisionEnd` / `OnCollision2DBegin` / `OnCollision2DEnd`
- `__annotations__` 类型注解自动暴露为编辑器公共字段（支持 float、int、bool、Vector2/3/4、Quaternion）
- **`Prism` Python 包**分层封装引擎 API：Core（Input/Time/Log/KeyCodes）、Math（Vector2/3/4、Quaternion、Matrix4、Mathf）、Renderer（预留）、Scene（预留）
- **CPython C API 原生桥接**：`PrismNative` 动态注册模块调用 C++ 引擎函数
- **PGLM 数学类型桥接**：`PythonMathBridge` 通过 Python Buffer Protocol 双向转换 glm ↔ pyglm 类型
- Python 标准库完整内置（Lib/ + DLLs/）

### 实体组件系统（ECS）
- 基于 **entt** 库
- 组件：`IDComponent`、`TagComponent`、`TransformComponent`、`MeshComponent`、`CameraComponent`、`SpriteRendererComponent`、`RigidBody2DComponent`、`BoxCollider2DComponent`、`CircleCollider2DComponent`、`BoxColliderComponent`、`SphereColliderComponent`、`CapsuleColliderComponent`、`RigidBodyComponent`
- 脚本组件：`PythonScriptComponent` 持有 `vector<PythonBehaviourBinding>`，`CSharpScriptComponent` 持有 `vector<CSharpBehaviourBinding>`
- 每个 BehaviourBinding 包含：BehaviourID（UUID）、ClassName、ModuleName、LifecycleMask（位掩码）、Fields（字段映射表）
- `LifecycleMask` 按位标记该脚本实现了哪些生命周期方法，运行时快速跳过空方法

### 现代 OpenGL 渲染管线
- PBR 金属/粗糙度工作流
- 计算着色器（Compute Shader）与 SSBO
- HDR 渲染 + MSAA 多重采样
- Prism Shader Language (PSL) — 自定义着色器语言，支持 Properties、RenderCommand、多 Pass
- **Shader 变体系统** — `#pragma shader_feature` + 自动编译 2^N 变体，`KeywordMask` 位掩码切换
- 模板测试封装与 Stencil Buffer 物体描边（Outline）
- **级联阴影映射（CSM）** — 4 级联、Practical Split Scheme 视锥分割、纹素对齐阴影稳定
- **PCF 软阴影** — 16 采样 Poisson Disk、旋转核去条纹、硬件 PCF 深度比较
- OpenGLStateCache 渲染状态统一管理
- RenderPass 系统与多线程命令队列
- Renderer2D + AABB 可视化调试
- Assimp 模型导入与子网格管理

### 编辑器功能（PrismEditor）
- ImGui 深度集成 + ImGuizmo 变换工具（平移/旋转/缩放）
- 场景层级面板 + 属性信息面板
- **多 Behaviour 脚本附件**：实体上可添加/移除多个 C# 和 Python Behaviour，编辑器弹出列表显示所有已注册脚本类
- **Behaviour 字段实时编辑**：`__annotations__` / C# 反射自动发现公共字段，生成 DragFloat、DragInt、Checkbox、DragFloat2/3/4 等 ImGui 控件
- 子网格拾取与高亮支持
- 运行时场景克隆与 Play/Pause 模式
- PBR 材质参数实时调节（反照率、法线、金属度、粗糙度）
- 包围盒可视化开关
- 2D 物理组件面板（RigidBody2D、BoxCollider2D、CircleCollider2D）
- 物理碰撞体调试绘制（Play 模式下渲染碰撞体边框）
- 材质面板（显示网格材质层级、着色器切换下拉框、关键字 Checkbox、属性实时编辑）
- Shader 关键字可视化开关（Material 面板自动列出所有 `#pragma shader_feature`）
- Physics2D 重力参数实时调节
- **阴影参数实时调节** — 阴影开关、深度偏移/法线偏移、级联数量
- ImGui Property 系统支持 Slider/Drag/Color 三种控件模式

### 基础系统
- 日志系统（spdlog，多级别、格式化、NativeString 跨语言）
- 相机系统（正交/透视/编辑器自由飞行）
- 输入系统（抽象层 + Windows 实现）
- Time 系统（模仿 Unity：DeltaTime、TimeScale、FrameCount）
- YAML 配置支持 + UUID 生成
- 侵入式 Ref 智能指针
- 数学库：GLM + AABB + Ray

## 项目结构

```bash
Prism/
├── Prism/                  # 引擎核心（C++17，编译为 DLL）
│   ├── src/Prism/
│   │   ├── Core/           # 应用框架、窗口、图层、日志、输入、Time、Ref、UUID
│   │   ├── Events/         # 事件系统（窗口、键盘、鼠标）
│   │   ├── Renderer/       # 渲染抽象层（RenderPass、Material、Mesh、Texture、Shader）
│   │   ├── Renderer/Camera/# 相机系统
│   │   ├── Scene/          # 场景管理与 ECS 组件
│   │   ├── Editor/         # 编辑器工具相机和层级面板
│   │   ├── ImGui/          # ImGui 集成层
│   │   ├── Debug/          # 性能分析工具
│   │   └── Utilities/      # 工具类
│   ├── src/Platform/
│   │   ├── OpenGL/         # OpenGL 后端实现（Buffer、FBO、Shader、Texture、SSBO 等）
│   │   └── Windows/        # Windows 平台层（GLFW 窗口、输入）
│   ├── src/Scripting/      # 多语言脚本引擎
│   │   ├── ScriptTypes.h   # 共享脚本类型定义（FieldType、LifecycleMethod、Metadata）
│   │   ├── Utility/        # 脚本工具（ScriptType 类型映射）
│   │   ├── CSharp/         # C++/C# 互操作引擎（Rolky、InternalCall 注册）
│   │   └── Python/         # Python 脚本引擎（CPython 嵌入、PrismNative 桥接）
│   │       ├── Interop/    # Python C API RAII 封装
│   │       │   ├── PythonScriptCore.h/.cpp  # ScriptHost、ScriptModule、ScriptClass、ScriptObject
│   │       │   └── PythonMathBridge.h/.cpp  # glm ↔ pyglm 双向转换
│   │       ├── PythonField.h             # 字段值绑定（Buffer + ScriptObject 指针）
│   │       ├── PythonScriptEngine.h/.cpp  # Python 运行时生命周期管理
│   │       ├── PythonScriptMetaRegistry.h/.cpp # 扫描 .py 文件注册 Behaviour 元数据
│   │       ├── PythonScriptStorage.h/.cpp   # 场景级 Python 脚本实例存储
│   │       └── PythonScriptWrappers.h/.cpp  # C++ 函数 → PrismNative Python 模块
│
├── Prism.Scripting/        # C# 脚本层（.NET 9 托管程序集）
│   └── src/Prism/
│       ├── Behaviour.cs    # Behaviour 基类（Enabled、Transform、GetComponent<T>）
│       ├── Entity.cs       # Entity 包装器（HasComponent/GetComponent/CreateComponent）
│       ├── Component.cs    # Component 基类 + 具体组件实现
│       ├── Core/           # Input、Time、Log、InternalCalls
│       ├── Math/           # Vector2/3/4、Quaternion、Matrix4、Mathf、Noise
│       ├── Renderer/       # Color、Material、Mesh、Texture2D
│       └── Scene/          # Entity、Component（TransformComponent 等）
│
├── PrismEditor/            # 编辑器应用程序（ImGui + ImGuizmo）
│   ├── src/                # PrismEditor.cpp、EditorLayer
│   └── Assets/scripts/Python/  # Python 用户脚本包
│       ├── Prism/          # 引擎 Python API（Entity、Behaviour、Core、Math）
│       │   ├── Entity.py         # Entity 包装器（HasComponent/GetComponent/CreateComponent）
│       │   ├── Behaviour.py      # Behaviour 基类（enabled、transform、GetComponent）
│       │   ├── Component.py      # Component 基类
│       │   ├── Core/             # Input、Time、Log、KeyCodes、Transform
│       │   └── Math/             # Vector2/3/4、Quaternion、Mathf
│       ├── SmokeTest.py    # API 冒烟测试
│       └── PrismNative.pyi # PrismNative 本机模块类型存根
│
├── ExampleApp/             # C# 示例脚本项目
│   └── src/                # Behaviour 子类示例：MapGenerator（程序化地形）、PlayerCube（2D 物理）、PlayerCube3D / PlayerSphere（3D 物理）、BasicController（相机移动）、Sink（下沉平台）、RandomColor（随机颜色）
├── SandBox/                # 旧版 C++ 示例（逐步淘汰）
├── vendor/                 # 第三方库
│   └── Python/             # CPython 3.13 嵌入式运行时（include、libs、DLLs、Lib/）
├── docs/                   # 技术文档（PSL、Time、Renderer）
├── premake5.lua            # 构建配置
├── Prism.sln               # Visual Studio C++ 解决方案
├── PrismManaged.sln        # Visual Studio C# 解决方案
└── Win-GenerateProjects.bat
```

## 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | C++17、C# 12 (.NET 9)、Python 3.13 |
| 图形 API | OpenGL 4.5+ (Core Profile) |
| 窗口 | GLFW |
| 数学 | GLM |
| ECS | entt（单头文件）|
| UI | ImGui + ImGuizmo |
| 脚本互操作 | Rolky、CPython 嵌入式 C API |
| 日志 | spdlog |
| 模型导入 | Assimp |
| 序列化 | yaml-cpp |
| 图像加载 | stb_image |
| 噪声 | FastNoise |
| 构建系统 | Premake 5 |
| 着色器 | Prism Shader Language (PSL) |

## 技术文档

- 中文版
  - [Prism Shader 文档](docs/PrismShader.md)
  - [Time 文档](docs/Time.md)
  - [Renderer 文档](docs/Renderer.md)
- English
  - [Time Documentation](docs/TimeEN.md)

## 开发路线图

- [x] **Entity-Behaviour 架构重构**（C#/Python 双语言 Behaviour 系统、多脚本挂载、生命周期统一）
- [x] 2D 物理系统集成（Box2D：刚体、碰撞体、重力）
- [x] **级联阴影映射（CSM）+ PCF 软阴影**
- [x] **Python 3.13 多语言脚本系统**（Prism Python 包、Behaviour 基类、PrismNative 原生桥接、PGLM 数学转换）
- [ ] 完整资源管理系统与序列化
- [ ] 跨平台窗口与渲染抽象
- [ ] Vulkan 后端支持（长期目标）

## 许可证

本项目采用 **Apache License 2.0** 许可证。详见 [LICENSE](LICENSE) 文件。

## 参考资料

- [Hazel Engine](https://github.com/TheCherno/Hazel)
- [LearnOpenGL CN](https://learnopengl-cn.github.io/)
- [entt](https://github.com/skypjack/entt)
- [ImGui](https://github.com/ocornut/imgui)
- [spdlog](https://github.com/gabime/spdlog)
