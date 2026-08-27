# Prism Engine

![Prism Engine Logo](https://github.com/HMsabachi/Prism/blob/main/Prism/assets/logo/WhiteLogo.png?raw=true "Prism Engine")

**Prism** 是一个轻量级、模块化的跨平台游戏引擎，核心使用 **C++20** 开发。渲染支持 **OpenGL 4.5+** 与 **Vulkan** 双后端，采用命令式多线程渲染架构（主线程录制、渲染线程执行），并基于 **GPU Scene（SSBO 实例数据）** 驱动场景绘制。内置 **PrismShaderCompiler** 自定义着色器编译器（支持 PSL → GLSL/HLSL/MSL/SPIR-V 多后端交叉编译），**3D 物理**集成 **NVIDIA PhysX**，**2D 物理**使用 **Box2D**。脚本系统支持 **C#**（.NET 9 + Rolky）和 **Python 3.13**（CPython 嵌入式），编辑器基于 **ImGui** 构建。

脚本系统采用 **Entity-Behaviour 架构**（类似 Unity），每个 Entity 可挂载多个 Behaviour 组件。C# 和 Python 脚本遵循统一的生命周期模型（`Awake` → `OnCreate` → `OnEnable` → `OnUpdate` → `LateUpdate` → `OnFixedUpdate` → `OnDisable` → `OnDestroy`），并支持 2D/3D 碰撞与触发回调。

> **注意：最新开发进度在 `Prism3D` 分支**（`git checkout Prism3D`）。

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

渲染后端与线程模型通过命令行参数选择：

```bash
PrismEditor.exe -r vulkan              # --renderer opengl | vulkan，Debug 默认 Vulkan，Release 默认 OpenGL
PrismEditor.exe -s                     # --singleThreaded 单线程渲染
```

## 截图 / 演示

![Prism Engine UI](docs/Screenshot/Editor2.png "Prism Engine UI")

![Prism Engine UI](docs/Screenshot/Editor3.png "Prism Engine UI")

## 特性一览

### PrismShaderCompiler — 自定义着色器编译器
- **PSL (Prism Shader Language)** — 自定义着色器语言，支持 Properties、RenderCommand、SubShader/Pass、Tags
- **多后端代码生成** — 从 PSL 统一源码交叉编译为 GLSL、HLSL、MSL、SPIR-V
- **反射元数据** — 编译时提取属性布局、Uniform 绑定、管线状态，运行时无需手动绑定
- **Shader 变体系统** — `#pragma shader_feature` 关键字 + 位掩码，自动编译 2^N 变体组合
- 集成在引擎运行时 — `PrismShader` 通过 `PrismShaderCompiler` 库加载并编译 `.Shader` 文件

### 多线程渲染架构
- 主线程录制渲染命令 -> **RenderCommandQueue** -> 渲染线程执行；OpenGL / Vulkan 双后端共用骨架，适配层各自实现
- 渲染状态操作全部经 `Renderer::Submit` 入队（无立即执行渲染状态），支持运行时切换单线程模式（`-s`）

### Vulkan 渲染后端
- 交换链（resize / 子最优重建 / 逐图像 RenderFinished 信号量）、渲染通道、帧缓冲、图形管线
- **PSO 管线缓存** - 管线状态对象化，状态命中不重建
- **VMA 显存管理** - UBO / SSBO 持久映射，分配池化管理
- **Frames-in-Flight（FIF=2）** - 每帧槽独立命令缓冲、UBO/SSBO 副本与描述符集，消除 CPU 写 / GPU 读竞争
- 全局描述符池 + **VulkanDescriptorSet** 动态绑定（绑定直传 + 反射校验布局）
- ImGui Vulkan 集成（二级命令缓冲 + 纹理描述符 set 缓存）
- **坐标语义统一** - 深度 z∈[0,1]（Vulkan 语义投影），GL 后端经 SPIRV-Cross `fixup_clipspace` 自动恢复 [-1,1]；front-face 与 FBO 采样语义双后端对齐，投影矩阵与 PSL 资产零改动通用

### GPU Scene 与绑定体系
- **GPU 实例数据** - `PrismObjects`（ObjectToWorld / PreviousModel / AnimationOffset）+ `PrismAnimation`（全骨骼矩阵）两个 std430 SSBO 每帧整块覆写，draw 只传 drawIndex（GL = uniform / Vulkan = push_constant），描述符不逐 draw 变化
- **三层绑定布局** - 全局层 set 0（相机 / 时间 / 灯光帧 UBO）+ Pass 自持输入（阴影图等）+ 材质自持（材质 UBO + 纹理）
- OpenGL 侧以 flat 绑定空间模拟同一语义，PSL 资产双后端通用

### 现代渲染管线（SceneRenderer）
- PBR 金属/粗糙度工作流 + HDR 渲染 + MSAA 8x 多重采样
- **级联阴影映射（CSM）** — 4 级联、Practical Split Scheme 视锥分割、纹素对齐、可配置最大阴影距离
- **PCF 软阴影** — 16 采样 Poisson Disk、旋转核去条纹、硬件 PCF 深度比较
- 计算着色器（Compute Shader）+ SSBO
- Geometry Pass（MSAA RGBA16F）→ Composite Pass（RGBA8 最终输出）
- RenderPass 系统
- 模板测试封装 + Stencil Buffer 物体描边（Outline）
- OpenGL State Cache 统一状态管理
- IBL 环境光照（辐照度 + 预过滤辐射度 + BRDF LUT）
- Assimp 模型导入与子网格管理

### C# 脚本系统（Entity-Behaviour 架构）
- .NET 9 运行时集成，基于 Rolky 的 C++/C# 互操作
- **多 Behaviour 支持**：同一 Entity 可挂载多个 C# Behaviour 子类
- 统一生命周期 + 碰撞/触发回调
- InternalCall 自动绑定机制（C# 端 `delegate* unmanaged[Cdecl]<>` 函数指针）
- 编辑器运行时脚本热重载（Play 时自动重载）
- C# 端完整 API：Core（Input / Time / Log）、Math、Renderer（Material / Mesh / Texture2D）、Physics

### Python 脚本系统（Entity-Behaviour 架构）
- Python 3.13 嵌入式 CPython 运行时
- **多 Behaviour 支持**：C# 和 Python Behaviour 可混合共存于同一 Entity
- 统一生命周期 + 2D/3D 碰撞回调（`OnCollisionBegin` / `OnCollisionEnd` / `OnCollision2DBegin` / `OnCollision2DEnd`）
- 类型注解字段自动暴露为编辑器公共字段（`float`、`Vector3`、`Mesh`、`Material`、`Texture2D` 等）
- **API 与 C# 端对称**：Core、Math、Renderer、Physics、Behaviour
- **pybind11 原生桥接**：`PrismEngine` C++ 模块提供完整 ECS + 资产 + 物理 API
- **glm ↔ pyglm 类型转换**：pybind11 类型 caster 自动双向转换向量/矩阵/四元数

### 物理系统
- **3D 物理（NVIDIA PhysX）**：
  - 刚体（静态/动态、质量、运动学、每轴锁定、Layer 过滤）
  - 碰撞体：Box、Sphere、Capsule、Mesh（Convex/Trimesh）
  - 物理材质：StaticFriction、DynamicFriction、Bounciness
  - 触发器 + 碰撞回调 + 射线/重叠检测
  - 编辑器碰撞体线框可视化
- **2D 物理（Box2D）**：
  - 刚体类型：Static、Dynamic、Kinematic
  - 碰撞体：Box、Circle
  - 重力参数实时调节

### 实体组件系统（ECS）
- 基于 **entt** 库，21 个组件类型：

| 组件 | 用途 |
|------|------|
| `IDComponent` | UUID 标识符 |
| `TagComponent` | 实体名称 |
| `TransformComponent` | 位置/旋转/缩放（含脏标志 + 矩阵分解） |
| `MeshRendererComponent` | 网格 + 子网格材质列表 |
| `CameraComponent` | SceneCamera + Primary 标志 |
| `SpriteRendererComponent` | 2D 精灵渲染 |
| `CSharpScriptComponent` / `PythonScriptComponent` | 脚本容器（多 Behaviour 映射） |
| `RigidBody2DComponent` / `BoxCollider2DComponent` / `CircleCollider2DComponent` | 2D 物理 |
| `RigidBodyComponent` / `PhysicsMaterialComponent` | 3D 物理刚体 + 材质 |
| `BoxColliderComponent` / `SphereColliderComponent` / `CapsuleColliderComponent` / `MeshColliderComponent` | 3D 碰撞体 |

### 资产管理系统
- 双管理器架构：`EditorAssetManager`（编辑器模式）+ `RuntimeAssetManager`（运行时模式）
- 资产注册表持久化 — 文件系统资产映射
- `ModelImporter` — Assimp 模型导入与网格/材质数据提取
- 13 种资产类型：Scene、Prefab、Mesh、Shader、Material、Texture、EnvMap、Font、ScriptFile、MeshCollider 等
- 物理资源（PhysicsMaterial、MeshCollider）资产化支持

### 场景系统
- `RenderSystem` — 拥有 SceneRenderer（全局单例），收集 FrameSnapshot 提交渲染，驱动每帧渲染、收集 MeshRenderer、提交调试绘制
- `ScriptSystem` — 桥接场景生命周期到 C#/Python 脚本引擎
- `PhysicsSystem` / `Physics2DSystem` — 3D/2D 物理场景管理
- `SceneCamera` — 透视/正交投影、可配置远近平面

### 编辑器功能（PrismEditor）
- ImGui 深度集成 + ImGuizmo 变换工具（平移/旋转/缩放）
- 场景层级面板 + 属性信息面板
- **多 Behaviour 脚本附件**：实体上可添加/移除多个 C# 和 Python Behaviour
- **Behaviour 字段实时编辑**：`__annotations__` / C# 反射自动发现公共字段
- 子网格拾取与高亮 + 包围盒可视化
- 运行时场景克隆与 Play/Pause/Stop 模式
- PBR 材质参数实时调节 + Shader 关键字可视化开关
- 材质面板（网格材质层级、着色器切换、属性实时编辑）
- **阴影参数实时调节** — 阴影开关、深度偏移/法线偏移、级联数量、最大阴影距离
- **3D 物理设置窗口** — 重力、物理层配置
- 2D 物理组件面板 + 碰撞体调试绘制
- ImGui Property 系统支持 Slider/Drag/Color 三种控件模式

### 基础系统
- 日志系统（spdlog，多级别、格式化、NativeString 跨语言）
- 相机系统（正交/透视/编辑器自由飞行）
- 输入系统（抽象层 + Windows 实现，含鼠标射线检测）
- Time 系统（模仿 Unity：DeltaTime、TimeScale、FrameCount、FixedDeltaTime）
- YAML 配置支持 + UUID 生成
- 侵入式 Ref 智能指针
- 数学库：GLM + AABB + Ray

## 项目结构

```bash
Prism/
├── Prism/                         # 引擎核心（C++20，编译为 DLL）
│   ├── src/Prism/
│   │   ├── Core/                  # 应用框架、窗口、图层、日志、输入、Time、Ref、UUID
│   │   ├── Events/                # 事件系统
│   │   ├── Renderer/              # 渲染抽象层（SceneRenderer、RenderPass、Material、Mesh、Shader）
│   │   │   ├── Buffer/            # Frame/Object UBO、SSBO、Framebuffer
│   │   │   ├── Camera/            # 相机系统
│   │   │   ├── Shader/            # PrismShader（PSL 编译）、ShaderVariant
│   │   │   └── ComputeShader/     # 计算着色器支持
│   │   ├── Scene/                 # 场景管理 + ECS 组件 + 系统（Render、Script、Physics）
│   │   ├── Physics/               # 3D 物理引擎（PhysX 包装、Layer、射线检测）
│   │   ├── Editor/                # 编辑器工具（EditorCamera、SceneHierarchyPanel、PhysicsSettingsWindow）
│   │   ├── Asset/                 # 资产管理系统（AssetManager、AssetRegistry、ModelImporter）
│   │   ├── ImGui/                 # ImGui 集成层
│   │   ├── Debug/                 # 性能分析工具
│   │   └── Utilities/             # 工具类
│   ├── src/Platform/
│   │   ├── OpenGL/                # OpenGL 后端（Buffer、FBO、Shader、Texture、SSBO 等）
│   │   ├── Vulkan/                # Vulkan 后端（Context、SwapChain、Pipeline、DescriptorSet、VMA、ImGuiLayer）
│   │   └── Windows/               # Windows 平台层（GLFW 窗口、输入）
│   ├── src/Scripting/             # 多语言脚本引擎
│   │   ├── CSharp/                # C++/C# 互操作（Rolky、InternalCall）
│   │   └── Python/                # Python 脚本引擎（pybind11 嵌入、PrismEngine 模块）
│   └── vendor/
│       ├── PrismShaderCompiler/   # 自定义着色器编译器（PSL 解析、多后端代码生成）
│       ├── PhysX/                 # NVIDIA PhysX 5.x SDK
│       └── Python/                # CPython 3.13 嵌入式运行时
│
├── Prism.Scripting/               # C# 脚本 API 层（.NET 9）
│   └── src/Prism/
│       ├── Behaviour.cs           # Behaviour 基类
│       ├── Entity.cs / Component.cs
│       ├── Core/                  # Input、Time、Log、InternalCalls
│       ├── Math/                  # Vector2/3/4、Quaternion、Matrix4、Mathf、Noise
│       ├── Renderer/              # Color、Material、Mesh、Texture2D、MeshFactory
│       └── Physics/               # Physics、Collider
│
├── PrismEditor/                   # 编辑器应用程序（ImGui + ImGuizmo）
│   ├── src/                       # PrismEditor.cpp、EditorLayer
│   └── Assets/
│       ├── Shaders/Engine/        # 引擎 GLSL 包含文件（PrismFrame、PrismShadow、PrismPBR）
│       └── Shaders/*.Shader       # PSL 着色器（PrismPBR、ShadowDepth 等）
│
├── ExampleApp/                    # C# 示例脚本项目
├── docs/                          # 技术文档（PSL、Renderer、Time、PythonScriptCore）
├── premake5.lua                   # Premake 5 构建配置
├── Prism.sln / PrismManaged.sln   # 解决方案文件
└── Win-GenerateProjects.bat
```

## 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | C++20、C# 12 (.NET 9)、Python 3.13 |
| 图形 API | OpenGL 4.5+ (Core Profile)、Vulkan |
| 显存管理 | VulkanMemoryAllocator (VMA) |
| 窗口 | GLFW |
| 数学 | GLM |
| ECS | entt（单头文件） |
| UI | ImGui + ImGuizmo |
| 3D 物理 | NVIDIA PhysX 5.x |
| 2D 物理 | Box2D |
| 着色器编译器 | PrismShaderCompiler（PSL → GLSL/HLSL/MSL/SPIR-V） |
| 脚本互操作 | Rolky（C#）、pybind11（Python） |
| 日志 | spdlog |
| 模型导入 | Assimp |
| 序列化 | yaml-cpp |
| 图像 I/O | stb_image |
| 噪声 | FastNoise |
| 构建系统 | Premake 5 |

## 技术文档

- [PSL 语法参考](Prism/vendor/PrismShaderCompiler/docs/PSL-Syntax.md) — 权威 PSL 语言规范
- [PrismShader 集成指南](docs/PrismShader.md) — 引擎侧 Shader 加载、变体管理、Pass 系统
- [PrismShader 扩展指南](docs/PrismShaderExtension.md) — 如何扩展着色器系统
- [Renderer 文档](docs/Renderer.md) — 渲染管线架构（SceneRenderer、CSM、PBR）
- [Python 脚本引擎文档](docs/PythonScriptCore.md) — pybind11 架构、PrismEngine 模块 API、扩展指南
- [Time 文档](docs/Time.md)
- [Time Documentation (English)](docs/TimeEN.md)
- [PrismShaderCompiler CLI 用法](Prism/vendor/PrismShaderCompiler/README.md)

## 开发路线图

- [x] **Entity-Behaviour 架构重构**（C#/Python 双语言 Behaviour 系统、多脚本挂载、统一生命周期）
- [x] **2D 物理系统（Box2D）**：刚体、碰撞体、重力
- [x] **PrismShaderCompiler**：PSL 自定义着色器语言 + 多后端交叉编译（GLSL/HLSL/MSL/SPIR-V）+ Shader 变体系统
- [x] **渲染管线重构**：SceneRenderer → RenderSystem + RenderPipeline、Geometry Pass + Composite Pass
- [x] **级联阴影映射（CSM）+ PCF 软阴影**：4 级联、PSSM 视锥分割、纹素对齐、可配置最大阴影距离
- [x] **Python 3.13 多语言脚本系统**：CPython 嵌入式、pybind11 桥接、PGLM 数学转换
- [x] **3D 物理系统（PhysX）**：刚体、4 种碰撞体、物理材质、触发器、碰撞回调、射线检测
- [x] **资产管理系统**：EditorAssetManager / RuntimeAssetManager、AssetRegistry、ModelImporter
- [x] **多线程渲染架构**：渲染线程 + RenderCommandQueue，OpenGL / Vulkan 双后端共用骨架
- [x] **Vulkan 渲染后端**：PSO 缓存、Frames-in-Flight、VMA 显存管理、全局描述符池、坐标语义统一
- [x] **GPU Scene 绑定体系**：三层绑定布局 + GPU 实例 SSBO + drawIndex
- [ ] 资产引用与完整序列化管线
- [ ] 跨平台窗口与渲染抽象

## 许可证

本项目采用 **Apache License 2.0** 许可证。详见 [LICENSE](LICENSE) 文件。

## 参考资料

- [Hazel Engine](https://github.com/TheCherno/Hazel)
- [LearnOpenGL CN](https://learnopengl-cn.github.io/)
- [entt](https://github.com/skypjack/entt)
- [ImGui](https://github.com/ocornut/imgui)
- [spdlog](https://github.com/gabime/spdlog)
- [NVIDIA PhysX](https://github.com/NVIDIA-Omniverse/PhysX)
