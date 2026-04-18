**Prism Shadeing Language (PSL) 使用文档**

---

### 1. 文件整体结构

```glsl
Shader "您的Shader名称"   // 必须是第一个有效语句，名称用于 ShaderLibrary 查找
{
    Properties
    {
        // 属性声明（见第2节）
    }

    RenderCommand
    {
        // 渲染状态命令（见第3节）
    }

    SubShader
    {
        Pass
        {
            Name "ForwardBase"          // Pass 名称（可选，但推荐）
            Tags { "Queue" = "Geometry" "LightMode" = "ForwardBase" }  // Tags（可选）

            GLSL
            {
                // 统一 GLSL 代码（必须同时写 main 和 frag，见第4节）
            }
        }

        // 可添加多个 Pass
    }
}
```

- **注释**：支持 `//` 单行和 `/* */` 多行，解析器 `StripComments` 会自动移除。
- **大小写敏感**：`Shader`、`Properties`、`RenderCommand`、`SubShader`、`Pass`、`GLSL` 必须完全匹配。
- **缩进与换行**：无严格要求，解析器使用 `ExtractBlock` 按 `{}` 匹配。

---

### 2. Properties 块（材质属性）

**语法**（每行一个属性）：
```glsl
变量名 ("显示名称", 类型) = 默认值
```

**支持的所有类型**（`StringToPropertyType` + `ParserPropertyType` 完整映射）：

| 类型写法                  | C++ 类型                  | GLSL Uniform 类型     | 默认值示例                  | 说明 |
|---------------------------|---------------------------|-----------------------|-----------------------------|------|
| `Color`                   | `Color` (vec4)            | `vec4`                | `(1,1,1,1)` 或 `white`     | 颜色 |
| `Float`                   | `Float`                   | `float`               | `0.5`                       | 浮点 |
| `Int`                     | `Int`                     | `int`                 | `1`                         | 整数 |
| `Vector2`                 | `Vector2`                 | `vec2`                | `(1,0)`                     | 二维向量 |
| `Vector3`                 | `Vector3`                 | `vec3`                | `(0,1,0)`                   | 三维向量 |
| `Vector4`                 | `Vector4`                 | `vec4`                | `(1,1,1,1)`                 | 四维向量 |
| `Texture2D`               | `Texture2D`               | `sampler2D`           | `white` 或任意字符串        | 2D纹理（slot自动分配） |
| `Texture2DMS`             | `Texture2D` (MS)          | `sampler2DMS`         | `white`                     | 多采样纹理 |
| `TextureCube`             | `TextureCube`             | `samplerCube`         | `white`                     | 立方体贴图 |
| `Range(min, max)`         | `Range`                   | `float`               | `0.5`                       | 带滑块范围 |
| `Matrix3` / `Matrix3X3`   | `Matrix3`                 | `mat3`                | `(1,0,0,0,1,0,0,0,1)`      | 3x3矩阵 |
| `Matrix4` / `Matrix4x4`   | `Matrix4`                 | `mat4`                | 单位矩阵                    | 4x4矩阵 |

**示例**：
```glsl
_MainColor ("主颜色", Color) = (1,1,1,1)
_Gloss ("光泽度", Range(0,1)) = 0.8
_MainTex ("基础贴图", Texture2D) = white
_NormalMap ("法线贴图", Texture2D) = bump
_Mat4 ("变换矩阵", Matrix4) = (1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1)
```

**引擎处理**：
- `ShaderProperty::Init` + `HandlePropertyValue` 会把默认值打包进 `m_DefaultValueBuffer`（`Buffer` 类）。
- 材质设置时调用 `PrismShader::SetProperty` 自动上传。
- `GetDefaultValue<T>(name)` 可在运行时读取默认值。

---

### 3. RenderCommand 块（渲染状态）

**语法**（`ParseShaderCommand` + `ShaderCommand.cpp` 逐 token 解析）：
```glsl
RenderCommand
{
    Blend Off                                      // 关闭混合（默认开启）
    Blend SrcAlpha OneMinusSrcAlpha                // RGB 混合因子
    Blend SrcAlpha OneMinusSrcAlpha One OneMinusSrcAlpha  // 分别指定 RGB/Alpha

    Cull Back                                      // 背面剔除（默认）
    Cull Front
    Cull Off

    ZTest LessEqual                                // 深度测试函数（默认 LessEqual）
    ZTest Off                                      // 关闭深度测试

    ZWrite On                                      // 开启深度写入（默认）
    ZWrite Off
}
```

**支持的所有命令**（完整枚举映射）：
- **Blend**：`Off` / `SrcAlpha OneMinusSrcAlpha` / `Zero One` 等（`BlendFactor` 枚举）
- **Cull**：`Back` / `Front` / `Off`（`CullMode`）
- **ZTest**：`LessEqual` / `Less` / `Greater` / `Equal` / `Always` / `Never` / `Off`（`DepthCompareFunc`）
- **ZWrite**：`On` / `Off`

**默认值**（`ShaderCommand` 结构体初始化）：
```cpp
flags = Blend | Cull | ZTest | ZWrite
cullMode = Back
zTestFunc = LessEqual
blendSrcRGB = SrcAlpha, blendDstRGB = OneMinusSrcAlpha
```

**引擎处理**：`PrismShader::Bind()` 时调用 `m_Shader->ApplyCommand(m_ShaderCommand)`，最终翻译为 OpenGL `glEnable(GL_BLEND)`、`glBlendFunc`、`glCullFace`、`glDepthFunc`、`glDepthMask` 等新版 API。

---

### 4. GLSL 块（核心着色器代码）

**一个 GLSL 块内必须同时写顶点和片元入口点**！

```glsl
GLSL
{
    // 引擎自动注入：
    // #version 450 core
    // uniform vec4 _MainColor;   // 所有 Properties
    // #ifdef PRISM_VERTEX_SHADER
    //     #define VARYING out
    // #else
    //     #define VARYING in
    //     layout(location = 0) out vec4 FragColor;
    // #endif

    // 您的属性声明（自动转为 layout(location = N) in）
    attribute vec3 aPos : POSITION;
    attribute vec3 aNormal : NORMAL;
    attribute vec2 aUV : TEXCOORD0;

    // 顶点着色器入口
    void main()
    {
        // PRISM_VERTEX_SHADER 被定义
        gl_Position = ...;
        VARYING vUV = aUV;   // 使用 VARYING 宏
    }

    // 片元着色器入口（必须叫 frag）
    void frag()
    {
        // PRISM_FRAGMENT_SHADER 被定义
        FragColor = texture(_MainTex, vUV);
    }
}
```

**引擎处理流程**（`ProcessAttributes` → `InjectHeader` → `SplitShader`）：
1. `ProcessAttributes`：把 `attribute ... : SEMANTIC` 转为 `layout(location = X) in`（语义映射见 `GetLocationBySemantic`）。
2. `InjectHeader`：注入版本、Properties Uniform、VARYING 宏。
3. `SplitShader`：
   - VS：保留 `main()`，删除 `frag()` 函数。
   - FS：删除 `main()`，把 `frag()` 重命名为 `main()`，删除所有 `layout(location) in` 属性。
4. 支持 `#include "Common.glsl"`（相对 `Assets/Shaders/Include` 目录，递归解析，防循环）。

**支持的 Vertex Semantic**（完整列表）：
`POSITION`、`NORMAL`、`TANGENT`、`BINORMAL`、`TEXCOORD0`、`TEXCOORD1`、`BONEINDICES`、`BONEWEIGHTS`、`INSTANCEID`、`COLOR`、`INDEX0`、`INDEX1`、`OTHER0`、`OTHER1`、`OTHER2`

---

### 5. 高级功能与注意事项

- **#include**：支持相对路径，`ResolveIncludes` 递归处理，历史记录防循环。
- **多 Pass**：`SubShader` 内可写多个 `Pass`，解析器会全部收集。
- **Tags**：目前仅解析存储在 `PassDescriptor::Tags`
- **Reload**：修改 .Shader 文件后调用 `PrismShader::Reload()` 会重新解析并触发所有 `ShaderReloadedCallback`。
- **运行时设置 Uniform**：使用 `PrismShader::SetInt`、`SetFloat`、`SetVec3`、`SetMat4` 等（直接调用底层 `Shader`）。
- **材质属性设置**：`material->SetProperty(shader.GetProperty().GetDefaultValueBuffer())` 或单独设置。

---

### 6. 完整最小示例

```glsl
Shader "Prism/Unlit"
{
    Properties
    {
        _MainColor ("主颜色", Color) = (1,1,1,1)
        _MainTex ("基础贴图", Texture2D) = white
    }

    RenderCommand
    {
        Blend SrcAlpha OneMinusSrcAlpha
        Cull Back
        ZTest LessEqual
        ZWrite On
    }

    SubShader
    {
        Pass
        {
            Name "ForwardBase"
            Tags { "Queue" = "Geometry" }

            GLSL
            {
                attribute vec3 aPos : POSITION;
                attribute vec2 aUV : TEXCOORD0;

                VARYING vec2 vUV;

                void main()
                {
                    gl_Position = vec4(aPos, 1.0);
                    vUV = aUV;
                }

                void frag()
                {
                    FragColor = texture(_MainTex, vUV) * _MainColor;
                }
            }
        }
    }
}
```

---

