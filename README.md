# Prism Engine

![Prism Engine Logo](https://github.com/HMsabachi/Prism/blob/main/Prism/assets/logo/WhiteLogo.png?raw=true "Prism Engine")

**Prism** 是一个轻量级、模块化、用 C++ 开发的跨平台游戏引擎，渲染后端使用 **OpenGL** （目前）。

目前处于早期开发阶段，已实现基础渲染管线、Shader 管理、Vertex Array 封装、日志系统和跨平台构建支持。目标是打造一个清晰、易扩展、高性能的引擎，适合学习和小型游戏开发。
## 🛠 快速开始

### 1. 克隆仓库（包含子模块）

```bash
git clone --recursive https://github.com/HMsabachi/Prism.git
cd Prism
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

打开生成的解决方案（或 Makefile），编译 `SandBox` 项目并运行，即可看到渲染结果。

## ⚙️ 规划与实现

- [ ] 资源管理系统
- [ ] 场景图与实体组件系统
- [ ] 物理系统集成
- [ ] 编辑器界面
- [ ] 2D 渲染管线 (正在进行)
- - [ ] 材质系统（正在进行） 
- - [ ] 重构Shader以支持**PSL**(*Prism Shader Language*)
- - [x] Transform
- [ ] 相机系统
- - [x] 正交相机
- [x] Time 系统
- [x] Shader 管理
- [x] Vertex Array 封装
- [x] 日志系统
- [x] 跨平台构建支持
## ✨ 特性（当前已实现）

- **OpenGL 现代渲染管线**  
  - OpenGL 上下文封装  
  - OpenGLShader 类 
  - Vertex Array / Vertex Buffer / Index Buffer 抽象  


- **数学库**：集成 GLM 数学库

- **日志系统**：底层使用 spdlog 统一日志输出，便于调试

- **构建系统**：使用 Premake 5，支持 Windows / Linux / macOS 快速生成项目文件 （但目前只有Windows支持）

- **示例项目**：SandBox 应用，用于快速测试引擎功能

- **ImGui** 初步集成（Sandbox 中已出现配置）

## 🧩 技术文档
- 中文版
- - [Prism Shader 文档](docs/PrismShader.md)
- - [Time 文档](docs/Time.md)
- - [Renderer 文档](docs/Renderer.md)
- English Version
- - [Time Documentation](docs/TimeEN.md)

## 📁 项目结构

```
Prism/
├── Prism/                  # 引擎核心源码
├── SandBox/                # 示例应用功能
│   └── src/                # Sandbox 主程序源码
├── vendor/                 # 第三方依赖
│   └── premake/            # Premake 5 构建工具
├── .gitignore
├── .gitmodules             # 子模块（当前主要包含 GLM）
├── GenerateProjects.bat    # Windows 一键生成项目文件
├── LICENSE
├── premake5.lua            # Premake 构建配置文件
└── README.md
```

> 具体代码实现建议直接查看仓库最新提交，核心渲染模块集中在 `Prism/Renderer/` 下。

## 🎯 开发路线图（Roadmap）

- [ ] 跨平台窗口抽象（GLFW 或自实现）
- [ ] 输入系统（键盘、鼠标）
- [ ] 2D 渲染管线（Sprite、Batch Rendering）
- [ ] 资源管理系统（Texture、Mesh、Material）
- [ ] 场景图与实体组件系统（ECS）
- [ ] 物理系统集成
- [ ] 编辑器界面（ImGui 扩展）
- [ ] Vulkan 后端支持（长期目标）

欢迎参与贡献，一起完善 Prism！

## 📸 截图 / 演示



## 🤝 贡献指南

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

欢迎任何形式的 Issue 和 PR！

## 📄 许可证

本项目采用 **Apache License 2.0** 许可证。  
详见 [LICENSE](LICENSE) 文件。

---

喜欢这个项目？欢迎点个 ⭐ 支持一下！

## 参考资料

- [LearnOpenGL CN](https://learnopengl-cn.github.io/)
- [spdlog](https://github.com/gabime/spdlog)    
- [Premake 5](https://premake.github.io/)
- [GLM](https://github.com/g-truc/glm)
- [ImGui](https://github.com/ocornut/imgui) 
