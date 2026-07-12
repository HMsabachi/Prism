# PrismShader — 引擎侧集成指南

Prism 使用 **PrismShaderCompiler** 独立编译器库处理 PSL (Prism Shader Language)。引擎通过 `PrismShader` 类包装编译器，管理编译产物的运行时加载、变体缓存和 Pass 查询。

> **PSL 语言语法参考请查阅权威文档：[PSL 语法参考](../Prism/vendor/PrismShaderCompiler/docs/PSL-Syntax.md)**

---

## 1. 架构概览

```
.Shader 文件（PSL 格式）
    │
    ▼
PrismShaderCompiler::ShaderCompiler::Compile()
    │  PSL 解析 → AST → 反射元数据
    ▼
CompiledShader（平台无关中间表示）
    ├─ ShaderName, LOD
    ├─ Uniforms[]          ← 材质属性列表
    ├─ Passes[]            ← Pass 信息（含 UsePass 深拷贝）
    ├─ Keywords[]          ← 变体关键字
    ├─ MaterialLayout      ← UBO 布局
    └─ RenderState         ← 全局渲染状态
    │
    ▼
PrismShader::Load()
    ├─ 调用 ShaderCompiler::Compile() 解析 PSL
    ├─ CompilePasses()：每个 Pass 调用 GenerateGLSL() → OpenGL 编译
    ├─ CompileVariants()：遍历 multi_compile 关键字组合，预编译变体
    └─ 触发 ReloadedCallbacks
    │
    ▼
Material / MaterialInstance
    ├─ 持有 PropertyBuffer
    ├─ Bind() 时通过 PrismShader::GetPassProgram(pass, mask) 选变体
    └─ SetKeyword() 控制关键字位
```

### 关键文件

| 文件 | 路径 | 职责 |
|------|------|------|
| PrismShader | `Renderer/Shader/PrismShader.h/.cpp` | 引擎侧 Shader 资产，包装编译器 + 变体管理 |
| ShaderLibrary | `Renderer/Shader/PrismShader.h/.cpp` | Shader 名称→实例映射，UsePass 解析回调 |
| ShaderCompiler | `ShaderCompiler/ShaderCompiler.h/.cpp` | 引擎内编译器配置与封装 |
| Compiler.h | `vendor/PrismShaderCompiler/PrismShaderCore/include/PrismShaderCore/Compiler.h` | 编译器公共 API |

---

## 2. PrismShader 核心 API

### 创建与加载

```cpp
// 从文件创建 Shader 资产（自动调用 Reload → Load）
Ref<PrismShader> shader = PrismShader::Create("Assets/Shaders/PrismPBR.Shader");

// 从源码字符串加载（编辑器即时编译等场景）
shader->Load(sourceString);

// 热重载（文件内容变更后调用）
shader->Reload();
```

`Load()` 内部流程：
1. 调用 `ShaderCompiler::Get().Compile(source, filePath)` 解析 PSL
2. 提取 ShaderName、Keywords、Uniforms
3. `CompilePasses()` — 每个 Pass 调用 `GenerateGLSL()` → `Shader::Create()` 编译基础程序
4. `CompileVariants()` — 为 `multi_compile` 关键字预编译所有组合

### 查询元数据

```cpp
const auto& compiled = shader->GetCompiledShader();

// Properties / Uniforms
for (auto& u : compiled.Uniforms)
    u.Name, u.Type, u.DefaultValue;

// Material UBO 布局
const auto& layout = compiled.MaterialLayout;
layout.Size, layout.Properties[];

// 关键字
for (auto& kw : compiled.Keywords)
    kw.Name, kw.IsMultiCompile;

// Pass
uint32_t count = shader->GetPassCount();
const ShaderPass& pass = shader->GetPass(i);
pass.Name, pass.Tags, pass.RenderState;

// 按 Tag 查找 Pass（如查找 ShadowCaster）
int32_t idx = shader->FindPassByTag(
    Hash::GenerateFNVHash64("LightMode"),
    Hash::GenerateFNVHash64("ShadowCaster"));
```

### 变体与渲染

```cpp
// 获取特定 Pass + 关键字组合的编译后 Shader 程序
KeywordMask mask = material->GetKeywordMask();
Ref<Shader> program = shader->GetPassProgram(passIndex, mask);
program->Bind();

// 关键字索引（构造 KeywordMask 时用）
uint8_t idx = shader->GetKeywordIndex("ALBEDO_MAP");
bool defined = shader->IsKeywordDefined("NORMAL_MAP");
```

### 热重载回调

```cpp
auto token = shader->AddShaderReloadedCallback([]() {
    // Shader 文件变更后触发，材质可在此重建 UBO 等
});
shader->RemoveShaderReloadedCallback(token);
```

---

## 3. ShaderLibrary — 全局 Shader 管理

```cpp
// 加载单个 Shader（自动注册到名称映射）
auto shader = shaderLib->Load("Assets/Shaders/PrismPBR.Shader");

// 批量加载目录下所有 .Shader 文件
shaderLib->LoadAll("Assets/Shaders");

// 按名称查找（名称来自 PSL 的 Shader "xxx" 声明）
Ref<PrismShader> shader = shaderLib->Get("Standard/PrismPBR");

// 检查是否存在
bool exists = shaderLib->Exists("Standard/PrismPBR");
```

`ShaderLibrary` 同时作为 `UsePass` 的回调解析器 — 当编译器遇到 `UsePass "Name" "Pass"` 时，通过 `OnResolveUsePass()` 查找目标 Shader 并返回其 `CompiledShader`。

---

## 4. 引擎内编译器配置

`ShaderCompiler`（`Prism/src/Prism/ShaderCompiler/`）封装了 `PrismShaderCompiler::ShaderCompiler`，提供引擎侧配置：

```cpp
// 引擎启动时初始化
ShaderCompiler::Init({
    .IncludeRoot = "Assets/Include",
    .EngineRoot  = "Assets/Engine",
});

// 获取编译器实例
auto& compiler = ShaderCompiler::Get();

// 按需生成 GLSL（变体编译）
auto output = compiler.GenerateGLSL(compiledShader, passIndex, keywords);
// output.VertexShader, output.FragmentShader
```

---

## 5. Shader 变体系统

### 两种关键字模式

| Pragma | 行为 | 编译时机 |
|--------|------|----------|
| `#pragma shader_feature KEY` | 按需编译 | Material 首次请求时（lazy） |
| `#pragma multi_compile KEY` | 全量预编译 | `PrismShader::Load()` 时（Debug/Release） |

> **注意**：Release/Dist 构建中 `shader_feature` 的按需编译避免了组合爆炸 — 仅编译实际用到的变体。

### KeywordMask

```cpp
using KeywordMask = uint64_t;  // 最多 64 个关键字
constexpr size_t MAX_KEYWORDS_PER_SHADER = 64;

// Material 侧
material->SetKeyword("ALBEDO_MAP", true);
material->SetKeyword("NORMAL_MAP", false);
KeywordMask mask = material->GetKeywordMask();
```

---

## 6. Pass 与 Tag 系统

每个 Pass 可携带 Tags，引擎通过 Tag 查找特定渲染 Pass：

```psl
Pass
{
    Name "ShadowCaster"
    Tags { "LightMode" = "ShadowCaster" }
    GLSL { ... }
}
```

C++ 侧查找：
```cpp
int32_t shadowPassIdx = shader->FindPassByTag(
    Hash::GenerateFNVHash64("LightMode"),
    Hash::GenerateFNVHash64("ShadowCaster"));
```

渲染器根据 Pass Tag 选择不同的渲染路径（Forward、ShadowCaster、DepthOnly 等）。

---

## 7. UsePass — Pass 复用

```psl
Shader "Custom/MyShader"
{
    SubShader
    {
        Pass { GLSL { ... } }
        UsePass "Standard/Forward" "ShadowCaster"
    }
}
```

`UsePass` 将目标 Shader 的 Pass（包括其 GLSL 代码、Tags、RenderState）深拷贝到当前 Shader。编译器通过 `ResolveUsePassCallback` 查找目标 — 引擎侧由 `ShaderLibrary::OnResolveUsePass()` 实现。

- 不支持循环引用
- 目标 Shader/Pass 不存在时编译报错

---

## 8. 与旧版（引擎内解析器）的主要差异

| | 旧版（已废弃） | 新版（PrismShaderCompiler） |
|---|---|---|
| 解析器位置 | 引擎内 `ShaderParser/Lexer/GLSLParser` | 独立库 `PrismShaderCompiler` |
| 顶点入口 | `void main()` | `void vert()` |
| 片元入口 | `void frag()` | `void frag()`（不变） |
| Varying | `VARYING` 宏 | `varying struct` 块 |
| 片元输出 | 自动注入 `FragColor` | 显式 `layout(location = N) out` |
| 多后端 | 仅 GLSL | GLSL / HLSL / MSL / SPIR-V |
| 关键字 | `#pragma shader_feature` | `shader_feature` + `multi_compile` |
| Pass 复用 | 无 | `UsePass` |
| LOD | 无 | `LOD` 值 |
| 渲染状态 | Blend/Cull/ZTest/ZWrite | 新增 ColorMask、Offset |

---

## 9. 扩展指南

### 添加新的 Property 类型

1. **PrismShaderCompiler 侧**：在 PSL 解析器中注册新类型的 AST 节点 + 代码生成逻辑
2. **引擎侧 `PrismShader.cpp`**：无需修改 — 编译器输出的 `ShaderUniform` 和 `MaterialLayout` 已包含所有信息
3. **编辑器 UI**：在 `EditorLayer.cpp` 的材质面板中添加对应的 ImGui 控件

### 添加新的 RenderCommand

1. **PrismShaderCompiler 侧**：在 `Pipeline/PipelineState.h` 添加字段 + PSL 解析器添加对应命令
2. **引擎侧**：在 `OpenGLShader.cpp` 的 `ApplyCommand()` 中添加对应的 OpenGL 状态调用

### 添加新的代码生成后端

编译器已支持 GLSL/HLSL/MSL/SPIR-V 生成。引擎侧只需：
```cpp
auto out = compiler.GenerateHLSL(compiledShader, passIndex, keywords);
// 或
auto out = compiler.GenerateSPIRV(compiledShader, passIndex, keywords);
```
