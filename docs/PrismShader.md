
---

### **PrismShader 技术文档 v1.0**

#### **1. 概述**

`PrismShader` 是 Prism 引擎的自定义材质着色器系统，采用类 Unity ShaderLab 的语法（Prism Shader Language v1.0），支持：

- 属性（Properties）声明与运行时编辑
- 多 Pass（当前仅支持单 Pass）
- GLSL 代码嵌入
- 材质参数自动解析与绑定（Float、Range、Int、Color、Vector2/3/4、Texture2D）
- 引擎内置变量（`Prism_` 前缀）

核心文件：
- `PrismShader.cpp` / `PrismShader.h`
- `PrismShaderParser.h/cpp`（解析器，仓库最新版为准）
- `Shader.cpp`（底层 OpenGL Shader 封装）

---

#### **2. Prism Shader Language 语法（v1.0）**

```glsl
// Prism Shader Language v1.0
Shader "分类/Shader名称"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        _变量名("显示名称", 类型) = 默认值
        // 支持类型：
        // Float、Range(min, max)、Int、Color、Vector2、Vector3、Vector4、Texture2D
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" }
            Name "ForwardBase"

            GLSL
            {
                #include "PrismBuiltin.glsl"   // 必须包含，获取引擎内置变量

                // 输入属性（attribute）
                attribute vec3 aPos : POSITION;
                attribute vec2 aUV : TEXCOORD0;
                attribute vec3 aNormal : NORMAL;

                // 跨阶段变量（VARYING）
                VARYING vec2 vUV;
                VARYING vec3 vNormal;

                void main()  // 顶点着色器
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(aPos, 1.0);
                    vUV = aUV;
                }

                void frag()  // 片元着色器（Prism 引擎会自动调用）
                {
                    // ... 你的着色逻辑
                    FragColor = vec4(1.0);
                }
            }
        }
    }
}
```

---

#### **3. 支持的 Property 类型**

| 类型          | 默认值示例                  | C++ 中对应类                          | 说明                  |
|---------------|-----------------------------|---------------------------------------|-----------------------|
| Float         | 0.5                         | `FloatPropertyElement`                | 浮点数                |
| Range(0,1)    | 0.5                         | `FloatPropertyElement`                | 带范围的浮点数        |
| Int           | 1                           | `IntPropertyElement`                  | 整数                  |
| Color         | (1,1,1,1)                   | `ColorPropertyElement`                | RGBA 颜色             |
| Vector2       | (1,0)                       | `Vec2PropertyElement`                 | 二维向量              |
| Vector3       | (1,1,0)                     | `Vec3PropertyElement`                 | 三维向量              |
| Vector4       | (1,1,1,1)                   | `Vec4PropertyElement`                 | 四维向量              |
| Texture2D     | "white" {}                  | `Texture2DPropertyElement`            | 2D纹理（目前默认ErrorTexture） |

**注意**：目前 `ConvertValue` 对 Texture2D 的默认值解析尚不完整（仅创建 ErrorTexture），后续可扩展支持 "white"、"black"、"normal" 等内置纹理。

---

#### **4. C++ 使用方式**

```cpp
// 加载 Shader（推荐从文件加载）
Ref<PrismShader> shader = PrismShader::Create("Assets/Shaders/MyShader.prism", true);

// 获取材质属性
auto& props = shader->GetProperties();

// 修改属性（支持运算符重载）
props["_MainColor"] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);     // Color
props["_Gloss"] = 0.8f;                                       // Float
props["_MainTex"] = someTexture;                              // Texture2D

// 获取原始 Shader 用于渲染
Ref<Shader> glShader = shader->GetOriginalShader();
```

---

#### **5. 引擎内置变量（PrismBuiltin.glsl）**

必须在 GLSL 中包含：

```glsl
// PrismBuiltin.glsl（引擎提供）
uniform mat4 Prism_Model;
uniform mat4 Prism_ViewProjection;
uniform vec4 Prism_Time;        // (Time, SinTime, CosTime, DeltaTime) 等，可自行扩展
// ... 其他全局变量后续添加
```

---

#### **6. 当前限制（v1.0）**

- 仅支持单个 Pass（`m_ParseResult.Passes[0]`）
- Texture2D 属性默认绑定 `Texture2D::ErrorTexture`
- 属性默认值解析对复杂格式支持有限（Vector 使用 `(x,y,z,w)` 或 `{x,y,z,w}`）
- 暂无多 Pass、LOD、Fallback、CustomEditor 等高级特性
- `frag()` 函数为 Prism 引擎自定义入口（非标准 `main()`）

---

### **样板 Shader（推荐模板）**

**文件：`StandardLit.prism`**

```glsl
// Prism Shader Language v1.0
Shader "Prism/StandardLit"
{
    Properties
    {
        _MainColor("主颜色", Color) = (1, 1, 1, 1)
        _MainTex("基础贴图 (Albedo)", Texture2D) = "white" {}
        _NormalTex("法线贴图", Texture2D) = "normal" {}
        _Metallic("金属度", Range(0, 1)) = 0.0
        _Smoothness("光滑度", Range(0, 1)) = 0.5
        _Cutoff("透明裁剪阈值", Range(0, 1)) = 0.5
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" }
            Name "ForwardBase"

            GLSL
            {
                #include "PrismBuiltin.glsl"

                attribute vec3 aPos : POSITION;
                attribute vec2 aUV : TEXCOORD0;
                attribute vec3 aNormal : NORMAL;
                attribute vec3 aTangent : TANGENT;

                VARYING vec2 vUV;
                VARYING vec3 vNormal;
                VARYING vec3 vWorldPos;

                void main()
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(aPos, 1.0);
                    
                    vUV = aUV;
                    vNormal = mat3(Prism_Model) * aNormal;   // 法线转换到世界空间
                    vWorldPos = (Prism_Model * vec4(aPos, 1.0)).xyz;
                }

                void frag()
                {
                    // 示例：简单颜色 + 时间闪烁（可替换为 PBR 计算）
                    vec4 albedo = texture(_MainTex, vUV) * _MainColor;
                    
                    // 简单光照模拟
                    vec3 lightDir = normalize(vec3(1.0, 1.0, -1.0));
                    float ndotl = max(dot(normalize(vNormal), lightDir), 0.0);
                    
                    vec3 color = albedo.rgb * (ndotl * 0.8 + 0.2);
                    
                    // 时间动画示例
                    color.r += sin(Prism_Time.x * 3.0) * 0.1;
                    
                    FragColor = vec4(color, albedo.a);
                }
            }
        }
    }
}
```

---
