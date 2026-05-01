

# Prism Engine

![Prism Engine Logo](https://github.com/HMsabachi/Prism/blob/main/Prism/assets/logo/WhiteLogo.png?raw=true "Prism Engine")

**Prism** 是一个轻量级、模块化、用 C++ 开发的跨平台游戏引擎，渲染后端使用 **OpenGL**（采用现代 OpenGL API，如 `glCreateBuffer`、`glCreateVertexArrays` 等）。

**注意：最新开发进度请切换到 `Prism3D` 分支**（`git checkout Prism3D`），所有最新提交均在此分支。

目前已实现现代渲染管线、Scene 系统、C# 脚本系统、ECS 架构、编辑器基础框架等功能。目标是打造一个清晰、易扩展、高性能的引擎，适合学习和中小型游戏开发。

## 🛠 快速开始

### 1. 克隆仓库并切换分支

```bash
git clone --recursive https://github.com/HMsabachi/Prism.git
cd Prism
git checkout Prism3D          # 切换到最新开发分支（必须）
```

### 2. 生成项目文件

- **Windows**：双击运行 `GenerateProjects.bat`（推荐）
- **其他平台**：手动执行

```bash
premake5 vs2022        # Windows Visual Studio
premake5 gmake         # Linux / macOS Makefile
premake5 xcode4        # macOS Xcode
```

### 3. 编译并运行

编译 `SandBox`（或 `ExampleApp`）项目并运行，即可看到最新的 Scene、Renderer2D、C# 脚本组件、编辑器界面等演示效果。

## 📸 截图 / 演示

![Prism Engine UI](docs/Screenshot/Editor2.png "Prism Engine UI")

![Prism Engine UI](docs/Screenshot/Editor3.png "Prism Engine UI")

## ⚙️ 规划与实现

- [ ] 完整资源管理系统（Texture、Mesh、Material 序列化）
- [ ] 物理系统集成
- [x] **C# 脚本系统**（已基本完善，支持组件化开发）
- [x] **实体组件系统（ECS）**（基于 entt，已初步实现）
- [x] 编辑器界面 (正在进行)
  - [x] PrismEditor 项目
  - [x] 场景层级面板 + 属性面板 + ImGuizmo
  - [x] 子网格拾取与高亮
- [x] 3D 渲染管线 (正在进行)
  - [x] 材质系统（Material/Instance）
  - [x] RenderPass 系统
  - [x] Compute Shader（多纹理与 PBR 预处理）
  - [x] SSBO 支持
  - [x] HDR 渲染 + MSAA 多重采样
  - [x] Prism Shader Language (PSL) 完整支持
- [x] 相机系统（已重构）
- [x] Renderer2D 模块（含 AABB 可视化调试）
- [x] Scene 系统（场景管理与对象生命周期）
- [x] 输入系统（已重构）
- [x] Time 系统与 YAML + UUID 支持
- [x] 日志系统（多级别、格式化、跨语言支持）

## ✨ 特性（当前已实现）

- **C# 脚本系统**（核心新特性）
  - C++/C# 互操作与反射（基于 Rolky）
  - .NET 运行时集成
  - 组件化脚本开发（ScriptComponent）
  - InternalCall 自动绑定机制

- **实体组件系统（ECS）**
  - 引入 **entt** 库
  - Entity-Component 架构初步完成

- **OpenGL 现代渲染管线**
  - 使用新版 OpenGL API
  - OpenGLShader（完整 PSL 支持）
  - RenderPass 系统
  - Compute Shader、SSBO、HDR、MSAA
  - OpenGLStateCache 渲染状态统一管理
  - Renderer2D + AABB 可视化

- **编辑器功能**
  - ImGui 深度集成 + ImGuizmo 变换工具
  - 场景层级面板 + 属性信息面板
  - 子网格拾取与高亮支持

- **基础系统**
  - 日志系统（重构后支持多级别、格式化、NativeString 跨语言）
  - 相机系统（已重构）
  - 输入系统（已重构）
  - YAML 配置支持 + UUID 生成类
  - 侵入式 Ref 智能指针

- **数学库**：集成 GLM
- **构建系统**：Premake 5（目前 Windows 支持最好）

## 🧩 技术文档
- 中文版
  - [Prism Shader 文档](docs/PrismShader.md)
  - [Time 文档](docs/Time.md)
  - [Renderer 文档](docs/Renderer.md)
- English Version
  - [Time Documentation](docs/TimeEN.md)

## 📁 项目结构

```bash
Prism/
├── Prism/                  # 引擎核心（Renderer、Scene 等）
├── Prism.Scripting/        # C# 脚本系统模块（最新）
├── PrismEditor/            # 编辑器模块（ImGui + ImGuizmo）
├── SandBox/                # 示例应用
├── ExampleApp/             # 另一个示例应用
├── vendor/                 # 第三方库（entt、Rolky、stb_image 等）
├── docs/
├── premake5.lua
├── GenerateProjects.bat
└── README.md
```



## 🎯 开发路线图（Roadmap）

- [ ] 完善 C# 脚本系统（热重载、编辑器脚本属性面板）
- [ ] 完整资源管理系统与序列化
- [ ] 物理系统集成
- [ ] 输入系统进一步完善（跨平台）
- [ ] 跨平台窗口与渲染抽象
- [ ] Vulkan 后端支持（长期目标）

欢迎参与贡献，一起完善 Prism！

## 🤝 贡献指南

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

欢迎任何 Issue 和 PR！

## 📄 许可证

本项目采用 **Apache License 2.0** 许可证。详见 [LICENSE](LICENSE) 文件。

---

喜欢这个项目？欢迎点个 ⭐ 支持一下！

## 参考资料
- [Hazel Engine](https://github.com/TheCherno/Hazel)
- [LearnOpenGL CN](https://learnopengl-cn.github.io/)
- [entt](https://github.com/skypjack/entt)
- [ImGui](https://github.com/ocornut/imgui)
- [spdlog](https://github.com/gabime/spdlog)



---

