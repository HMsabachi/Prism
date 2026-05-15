# Prism Engine - 渲染模块技术文档（Renderer）

**版本**：早期开发阶段（2026年3月）  
**渲染后端**：OpenGL  
**更新日期**：2026年3月25日

本文档记录当前渲染系统的所有核心类、主要方法和设计思路，防止接口遗忘。建议每次重构渲染模块后同步更新此文档。

## 1. 渲染架构总览

Prism 采用**抽象渲染层**设计，便于未来扩展 Vulkan / DirectX 等后端。

核心类层级关系：
- `Renderer`：高级渲染命令入口（静态类）
- `RendererCommand`：底层命令分发
- `RendererAPI`：抽象接口（OpenGLRendererAPI 为当前实现）
- `Shader`：着色器管理
- `VertexArray` / `VertexBuffer` / `IndexBuffer`：缓冲区抽象
- `OpenGLContext`：OpenGL 上下文管理

渲染流程（当前）：
`Application` → `Layer` → `Renderer::Submit(...)` → `RendererCommand` → `RendererAPI` → OpenGL 调用

## 2. 核心类及接口

### 2.1 Renderer（渲染总入口）

位置：`Prism/Prism/Renderer/Renderer.h`（预计路径，根据你实际调整）

```cpp
// 静态类，供外部方便调用
class Renderer
{
public:
    static void Init();                                      // 初始化渲染器（创建API实例、设置默认状态等）
    static void Shutdown();                                  // 清理渲染器资源
// ------------------- 以上还未实现

    static void BeginScene(OrthographicCamera& camera);       // 开始一帧渲染场景（设置投影、视图矩阵等）
    static void EndScene();                                  // 结束场景（可用于批量提交绘制）

    // 提交绘制命令
    static void Submit(const std::shared_ptr<Shader>& shader,
                       const std::shared_ptr<VertexArray>& vertexArray,
                       const glm::mat4& transform = glm::mat4(1.0f));

    static RendererAPI::API GetAPI();                        // 获取当前使用的渲染API类型
};
```

**使用示例**（在 Sandbox Layer 中）：
```cpp
Renderer::BeginScene(camera);
Renderer::Submit(m_Shader, m_VertexArray, transform);
Renderer::EndScene();
```

### 2.2 RendererCommand

位置：`Prism/Prism/Renderer/RendererCommand.h`

```cpp
class RendererCommand
{
public:
    static void SetClearColor(const glm::vec4& color);
    static void Clear();
    static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0);
    // 未来可扩展：DrawLines、SetViewport、SetDepthTest 等
};
```

### 2.3 RendererAPI（抽象接口）

位置：`Prism/Prism/Renderer/RendererAPI.h`

```cpp
class RendererAPI
{
public:
    enum class API
    {
        None = 0,
        OpenGL = 1
        // Vulkan, Direct3D12, Metal...
    };

    virtual ~RendererAPI() = default;

    virtual void Init() = 0;
    virtual void SetClearColor(const glm::vec4& color) = 0;
    virtual void Clear() = 0;
    virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

    static API GetAPI() { return s_API; }   // 当前全局API类型

private:
    static API s_API;   // 默认为 OpenGL
};
```

**当前实现**：`OpenGLRendererAPI`（继承自 RendererAPI）

### 2.4 Shader

位置：`Prism/Prism/Renderer/Shader.h`

```cpp
class Shader
{
public:
	Shader(const std::string& VertexShader, const std::string& FragmentShader);

	~Shader();

	void Bind() const;
	void Unbind() const;
public:
	void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

};
```

**OpenGL 实现**：`OpenGLShader`

### 2.5 VertexArray（顶点数组对象）


```cpp
class VertexArray
{
public:
    static std::shared_ptr<VertexArray> Create();

    virtual ~VertexArray() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

    virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
    virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;
};
```

### 2.6 VertexBuffer & IndexBuffer

```cpp
class VertexBuffer
{
public:
    static std::shared_ptr<VertexBuffer> Create(float* vertices, uint32_t size);   // 静态

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetData(const void* data, uint32_t size) = 0;

    virtual const BufferLayout& GetLayout() const = 0;
    virtual void SetLayout(const BufferLayout& layout) = 0;
};

class IndexBuffer
{
public:
    static std::shared_ptr<IndexBuffer> Create(uint32_t* indices, uint32_t count);

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual uint32_t GetCount() const = 0;
};
```

**BufferLayout & BufferElement**（很重要，用于描述顶点格式）：
- `ShaderDataType`（Float, Float2, Float3, Float4, Mat3, Mat4, Int 等）
- `GetComponentCount()`、`GetSize()` 等辅助函数

### 2.7 OpenGLContext

```cpp
class OpenGLContext
{
public:
    OpenGLContext(GLFWwindow* windowHandle);

    void Init();
    void SwapBuffers();
};
```
### 2.8 Camera（摄像机）

位置：`Prism/Prism/Renderer/Camera/OrthographicCamera.h`
```cpp
class PRISM_API OrthographicCamera
{
public:
	OrthographicCamera(float left, float right, float bottom, float top);


	const glm::vec3& GetPosition() const;
	void SetPosition(const glm::vec3& position) ;
	float GetRotation() const ;
	void setRotation(float rotation) ;

	const glm::mat4& GetProjectionMatrix() const ;
	const glm::mat4& GetViewMatrix() const ;
	const glm::mat4& GetViewProjectionMatrix() const ;
}
```

## 3. 当前已知待完善 / 容易忘记的点

- `Renderer::BeginScene` / `EndScene` 目前是否完全实现？建议加入 Camera 支持。
- Uniform 设置是否全部在 OpenGLShader 中实现？（尤其是 Mat4、Vector 系列）
- VertexArray 是否已正确处理多个 VertexBuffer + 一个 IndexBuffer？
- 是否添加了 `Renderer2D` 高级接口（DrawQuad 等）？建议尽快加上，方便 Sandbox 测试。
- 纹理（Texture2D）接口还未添加，推荐下阶段实现：
  ```cpp
  class Texture2D { ... Bind(), SetData(), LoadFromFile() ... };
  ```

## 4. 使用注意事项

1. 所有渲染资源都建议使用 `std::shared_ptr` 管理。
2. 绘制前必须 `Shader->Bind()` 和 `VertexArray->Bind()`。
3. Clear 操作统一通过 `RendererCommand::Clear()`。
4. 以后添加 Vulkan 后端时，只需新增 `VulkanRendererAPI` 继承 RendererAPI 即可，上层代码无需修改。

## 5. 阴影系统

### 5.1 架构概览

Prism 使用**级联阴影映射（CSM）**实现方向光阴影，最大支持 4 级联，每级联 2048×2048 深度贴图。

```
SceneRenderer::FlushDrawList()
  ├── SceneRenderer::ShadowPass()       ← 渲染深度到 ShadowFBO
  ├── SceneRenderer::GeometryPass()     ← 绑定深度纹理 + 渲染实体（采样阴影）
  └── SceneRenderer::CompositePass()
```

### 5.2 ShadowPass（深度渲染）

位置：`Prism/src/Prism/Renderer/SceneRenderer.cpp`

```
for each cascade:
  Bind ShadowFBO[i]
  Clear depth
  Set u_LightVP = ShadowMatrices[cascade]
  for each mesh:
    SubmitMesh(mesh, shadowDepthMaterial)
```

- 使用 Depth-only FBO（`FramebufferFormat::Depth`），无颜色附件
- 着色器 `ShadowDepth.Shader`：仅输出 `gl_Position = u_LightVP * Prism_Model * a_Position`
- 深度纹理创建参数：`GL_DEPTH_COMPONENT32` + `GL_COMPARE_REF_TO_TEXTURE` + `GL_LEQUAL`（硬件 PCF 比较模式）

### 5.3 CSM 级联分割

位置：`UpdateShadowData()` in `SceneRenderer.cpp`

使用 **Practical Split Scheme** 混合对数/均匀分割：

```cpp
float splitLambda = 0.95f;
float logSplit = nearClip * pow(farClip / nearClip, fraction);
float uniSplit = nearClip + (farClip - nearClip) * fraction;
splits[i] = logSplit * splitLambda + uniSplit * (1.0f - splitLambda);
```

每个级联的步骤如下：
1. 构建相机视锥体 8 顶点（观察空间） → 转换到世界空间
2. 计算视锥中心 → 构建光照空间视图矩阵 `lightView`
3. 将 8 顶点转换到光照空间 → 计算 AABB
4. **纹素对齐**：对齐正交投影边界到纹素网格，消除阴影边缘抖动
5. 生成 `ShadowMatrices[cascade] = ortho(...) * lightView`

### 5.4 PCF 软阴影

位置：`PrismEditor/Assets/Shaders/Include/PrismBuiltin.glsl`

使用 **16 采样 Poisson Disk PCF**：

```glsl
// 16 预计算泊松盘样本 + 伪随机旋转核（去条纹）
float angle = Prism_Hash(projCoords.xy) * PI;
mat2 rot = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));

float shadow = 0.0;
for (int i = 0; i < 16; i++) {
    vec2 offset = rot * POISSON_DISK[i] * texelSize * filterRadius;
    shadow += texture(shadowMap, vec3(projCoords.xy + offset, depth - bias));
}
shadow /= 16.0;
```

- `sampler2DShadow` 提供硬件深度比较 + 双线性插值
- `filterRadius = 2.5` 纹素，可通过 `Prism_ShadowParams.w` 参数化
- 每片段伪随机旋转核，消除规则采样条纹

### 5.5 全局 Uniform 数据

位置：`Prism/src/Prism/Renderer/Shader/GlobalUniforms.h`

```cpp
glm::mat4 ShadowMatrices[4]{};     // 每级联的光照 VP 矩阵
glm::vec4 CascadeSplits{};         // 级联分割距离（视空间 z）
glm::vec4 ShadowParams{};          // x=深度偏移, y=法线偏移, z=级联数, w=未使用
```

绑定到 UBO binding 0，着色器侧对应 `Prism_ShadowMatrices`、`Prism_CascadeSplits`、`Prism_ShadowParams`。

### 5.6 编辑器控制

位置：`PrismEditor/src/EditorLayer.cpp`

在 Light 面板下提供：
- **Shadows** 开关
- **Shadow Bias**（0~0.05，深度偏移抗自阴影）
- **Normal Bias**（0~1，法线偏移抗自阴影）
- **Cascades**（1~4，级联数量）

所有参数在 Scene 中持久化（yaml-cpp 序列化）。

### 5.7 注意事项

1. 阴影 FBO 使用 `GL_CLAMP_TO_EDGE`，超出 [0,1] 范围的投影坐标不会产生边缘泄漏
2. 深度纹理比较模式 `GL_COMPARE_REF_TO_TEXTURE` 使 `sampler2DShadow` 的 `texture()` 返回比较结果而非深度值
3. 自阴影通过 ShadowBias（深度偏移）和 NormalBias（沿法线偏移世界坐标）双重控制
4. CSM 当前对所有 Mesh 生效，不支持单独设置投射/接收阴影标志

---

