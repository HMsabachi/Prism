# PrismShader 扩展指南

本文档说明如何修改和扩展 Prism Shader 系统的各个组件，按修改的常见需求分类。

---

## 1. 架构概览

```
.Shader 文件（PSL 格式）
    │
    ▼
ShaderParser → Lexer → GLSLParser
    │                       │
    │            ┌──────────┴──────────┐
    │            ▼                    ▼
    │    ParsePragmas()    ProcessAttributes()
    │    (提取关键字)       (语义→location)
    │            │                    │
    └──────┬─────┴────────────────────┘
           ▼
    ParseResult
    ├─ ShaderName
    ├─ Properties[]      ← 材质属性声明
    ├─ RenderCommand     ← 渲染状态
    ├─ Passes[]          ← VS/FS 源码
    └─ Keywords[]        ← shader_feature 关键字
           │
           ▼
    PrismShader::Load()
    ├─ 创建基础 Shader 程序
    ├─ 编译 2^N 个关键字变体
    ├─ 填充 PropertyDeclaration
    ├─ 打包默认值到 Buffer
    └─ 触发 ReloadedCallbacks
           │
           ▼
    Material / MaterialInstance
    ├─ 持有 PropertyBuffer
    ├─ Bind() 时选变体 + 上传 Uniform
    └─ SetKeyword() 控制关键字位
```

**关键文件**：

| 文件 | 路径 | 职责 |
|------|------|------|
| ShaderParser | `Renderer/Shader/Parser/ShaderParser.cpp` | PSL 语法解析入口 |
| Lexer | `Renderer/Shader/Parser/Lexer.cpp` | 词法分析（Token 化） |
| GLSLParser | `Renderer/Shader/Parser/GLSLParser.cpp` | GLSL 内部解析（属性、Varying、Pragma、函数） |
| ShaderParserData | `Renderer/Shader/Parser/ShaderParserData.h` | 解析结果数据结构 |
| PrismShader | `Renderer/Shader/PrismShader.cpp` | 核心着色器类，加载、编译、变体管理 |
| ShaderPropertyDeclaration | `Renderer/Shader/ShaderPropertyDeclaration.h` | 属性声明与缓冲区布局 |
| ShaderCommand | `Renderer/Shader/ShaderCommand.cpp` | 渲染状态命令解析 |
| ShaderVariant | `Renderer/Shader/ShaderVariant.h` | 变体数据结构（Mask + ShaderProgram） |
| Material | `Renderer/Material.cpp` | 材质与材质实例，关键字控制 |
| ScriptWrappers | `Scripting/ScriptWrappers.cpp` | C# 互操作包装器 |

---

## 2. 扩展 Property 类型

### 步骤

**Step 1：在枚举中添加新类型**

`ShaderPropertyDeclaration.h` → `PropertyDeclarationType` 枚举：
```cpp
enum class PropertyDeclarationType : uint32_t
{
    Bool, Float, Int, Vector2, Vector3, Vector4,
    Color, Color3, Range, Enum,
    Texture2D, Texture2DMS, TextureCube,
    Matrix3, Matrix4,  // ← 已有
    // 添加新类型：
    Double,             // ← 新增
};
```

**Step 2：在 C++ 类型映射中添加对应类型**

`Utilities.h` → `PropertyType` 命名空间：
```cpp
namespace PropertyType {
    using Double = double;  // ← 新增
    // ...
}
```

**Step 3：在解析器中添加映射**

`ShaderParser.cpp` → `StringToPropertyType()`：
```cpp
if (type == "Double") return PropertyDeclarationType::Double;
```

**Step 4：在默认值打包中添加处理**

`PrismShader.cpp` → `PackDefaultValues()`：
```cpp
case PropertyDeclarationType::Double:
{
    Double val = Parse<Double>(prop.DefaultValue);
    m_DefaultValueBuffer.Write((byte*)&val, sizeof(Double), offset);
    break;
}
```

**Step 5：在编辑器 Material 面板中添加 UI**

`EditorLayer.cpp` → `DrawMaterialProperty()`：
```cpp
case PropertyDeclarationType::Double:
{
    auto& value = materialInstance.Get<Type::Double>(name);
    ImGui::InputDouble(("##" + name).c_str(), &value);
    break;
}
```

**Step 6：在 C# 端（如果需要）暴露新的 Set/Get**

- `Material.cs`：添加 `Set(string uniform, double value)` 方法
- `ScriptWrappers.cpp`：添加对应的 InternalCall
- `ScriptEngineRegistry.cpp`：注册 InternalCall
- `InternalCalls.cs`：声明 delegate

---

## 3. 扩展 RenderCommand

### 步骤

**Step 1：在枚举中添加命令**

`ShaderCommand.h` → `ShaderCommandType` 或对应枚举。

**Step 2：在解析器中添加解析**

`ShaderCommand.cpp` → `ParseShaderCommand()`：
```cpp
else if (token == "Wireframe")
{
    command.wireframe = true;
}
```

**Step 3：在 ApplyCommand 中添加 OpenGL 调用**

`OpenGLShader.cpp`（或对应后端）：
```cpp
if (command.wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
```

---

## 4. 扩展 Shader 关键字系统

### 添加新关键字

在 `.Shader` 文件的 GLSL 块中添加：
```glsl
#pragma shader_feature MY_NEW_FEATURE
```

引擎自动处理：
- `ParsePragmas()` 提取 `MY_NEW_FEATURE`
- 编译 2 个变体（启用/未启用）
- Material 面板自动显示 Checkbox

### 修改变体编译逻辑

`PrismShader.cpp` → `Load()` 中的变体编译循环：
```cpp
// 编译上限：10 个关键字 = 1024 个变体
if (kwCount > 10) {
    PR_CORE_WARN("...skipping...");
}
```

如需调整上限，修改 `ShaderVariant.h` 中的：
```cpp
constexpr size_t MAX_KEYWORDS_PER_SHADER = 64;
```

### 自定义变体生成策略

如需改变变体生成方式（如只编译特定组合），修改 `PrismShader::Load()` 中的循环：
```cpp
for (KeywordMask mask = 1; mask < numVariants; mask++)
{
    // 可在此处跳过不需要的组合
    if (mask & SOME_SKIP_MASK) continue;
    // ...
}
```

---

## 5. 修改 GLSL 代码生成

### 修改属性声明生成

`GLSLParser.cpp` → `ProcessAttributes()`：
- 控制 `attribute` → `layout(location = N) in` 的转换
- 修改 `GetLocationBySemantic()` 调整 location 分配

### 修改 Header 注入

`ShaderParser.cpp` → `InjectHeader()`：
- 控制 `#version` 声明
- 控制 VARYING 宏定义
- 控制 FragColor 输出声明

### 添加新的预处理指令

`GLSLParser.cpp` → `ParsePragmas()` 支持新指令类型：
```cpp
static const std::regex multiCompileRegex(R"prism(multi_compile\s+(.+))prism");
static const std::regex shaderFeatureRegex(R"prism(shader_feature\s+(.+))prism");
// 添加：
static const std::regex myPragmaRegex(R)prism(my_pragma\s+(.+))prism");
```

然后在 `GLSLParser.h` 的 `GLSLPragma` 结构体中添加对应字段。

---

## 6. 添加新的 Pass 类型

`.Shader` 文件中定义：
```glsl
SubShader
{
    Pass
    {
        Name "ShadowMap"
        Tags { "LightMode" = "Shadow" }
        GLSL { /* ... */ }
    }
}
```

引擎自动收集到 `ParseResult::Passes[]`，每个 Pass 独立编译 VS/FS。
需添加逻辑的地方：
- `SceneRenderer.cpp`：根据 Pass 名称或 Tag 选择渲染路径
- `PrismShader.cpp`：为每个 Pass 保存源码用于变体编译

---

## 7. C# 互操作扩展流程

如果新增的 C++ API 需要在 C# 端暴露，按以下步骤：

1. **ScriptWrappers.cpp**：实现 InternalCall 函数
2. **ScriptWrappers.h**：添加函数声明
3. **ScriptEngineRegistry.cpp**：注册 InternalCall（`PR_ADD_INTERNAL_CALL`）
4. **InternalCalls.cs**：声明 `delegate* unmanaged[Cdecl]<...>` 字段
5. **对应的 C# 类**（Material.cs / Mesh.cs 等）：调用 InternalCalls

---

## 8. 常见扩展场景速查

| 需求 | 修改位置 | 涉及文件 |
|------|----------|----------|
| 添加新 Property 类型 | 枚举 + 解析 + PackDefaultValues + UI | `ShaderPropertyDeclaration.h`, `ShaderParser.cpp`, `PrismShader.cpp`, `EditorLayer.cpp` |
| 添加新 RenderCommand | 解析 + ApplyCommand | `ShaderCommand.cpp`, `OpenGLShader.cpp` |
| 添加 shader_feature 关键字 | 只需改 .Shader 文件 | `*.Shader` |
| 修改变体编译上限 | 更改常量 | `ShaderVariant.h` |
| 添加新 Vertex Semantic | GLSLParser 映射 | `GLSLParser.cpp` |
| 添加新 Pass 类型 | .Shader 文件 + 渲染器逻辑 | `*.Shader`, `SceneRenderer.cpp` |
| 暴露 C++ API 到 C# | 4 个文件修改 | `ScriptWrappers.cpp/.h`, `ScriptEngineRegistry.cpp`, `InternalCalls.cs` |
