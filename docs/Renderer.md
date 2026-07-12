# Prism Engine — 渲染模块技术文档

**渲染后端**：OpenGL 4.5+ Core Profile
**更新日期**：2026年7月

本文档记录渲染系统的当前架构、核心类和渲染流程。

---

## 1. 渲染架构总览

Prism 采用 **RenderPipeline + RenderSystem** 架构，渲染流程由 `RenderSystem`（ECS System）驱动，`RenderPipeline` 执行多 Pass 渲染。

```
Scene::OnRender(dt)
  └── RenderSystem::OnRender(dt)
        ├── CollectMeshRenderers(snapshot)   ← 遍历 ECS 收集 MeshRenderer
        ├── CollectDebugDraws(snapshot)      ← 收集调试绘制
        └── RenderPipeline::Execute(snapshot)
              ├── BeginFrame()               ← 上传 Per-Frame UBO
              ├── UpdateShadowData()         ← CSM 级联分割
              ├── ShadowPass()               ← 4 级联深度渲染
              ├── GeometryPass()             ← PBR 渲染 + 选中描边 + 调试线框
              ├── BloomBlurPass()            ← 双 Pass 高斯模糊
              ├── BloomBlendPass()           ← 混合 Bloom 到场景
              └── CompositePass()            ← MSAA Resolve → 最终输出
```

---

## 2. 核心类

### 2.1 RenderSystem（ECS System）

位置：`Prism/src/Prism/Scene/Systems/RenderSystem.h`

```cpp
class RenderSystem : public ISystem
{
    void OnRender(float dt) override;    // 每帧渲染入口
    void SetEditorCamera(const EditorCamera& camera);
    void SetViewportSize(uint32_t width, uint32_t height);
    void SubmitDebugMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<Material> material);
    Ref<RenderPass> GetFinalRenderPass();
    RenderConfig& GetConfig();
};
```

### 2.2 RenderPipeline

位置：`Prism/src/Prism/Renderer/RenderPipeline.h`

```cpp
class RenderPipeline
{
    void Initialize(uint32_t viewportWidth, uint32_t viewportHeight);
    void Execute(const FrameSnapshot& snapshot);  // 执行完整渲染管线

    Ref<RenderPass> GetFinalRenderPass() const;
    const Ref<RenderPass>& GetShadowPass(uint32_t index) const;  // 0~3
};
```

### 2.3 FrameSnapshot — 渲染数据载体

```cpp
struct FrameSnapshot
{
    RendererCamera Camera;
    RenderConfig Config;
    std::vector<DrawCommand> DrawList;         // 渲染物体
    std::vector<DrawCommand> SelectedDrawList; // 选中物体
    std::vector<DrawCommand> ShadowDrawList;   // 投射阴影的物体
    std::vector<DrawCommand> DebugDrawList;    // 调试绘制
};
```

### 2.4 DrawCommand

```cpp
struct DrawCommand
{
    Ref<Mesh> Mesh;
    uint32_t SubmeshIndex = 0;
    Ref<Material> Material;
    glm::mat4 Transform;
    uint64_t SortKey = 0;  // 排序键（不透明/透明、材质等）
};
```

### 2.5 PrismShader — Shader 资产

位置：`Prism/src/Prism/Renderer/Shader/PrismShader.h`

通过 `PrismShaderCompiler` 编译 PSL，管理 Pass 和变体。详见 [PrismShader 集成指南](PrismShader.md)。

### 2.6 Material / MaterialInstance

```cpp
// 从 PrismShader 创建
Ref<Material> mat = Material::Create(shader->Handle);

// 属性设置
mat->SetFloat("_Metallic", 0.8f);
mat->SetVec3("_Color", glm::vec3(1,0,0));
mat->SetTexture("_MainTex", texture);

// 关键字
mat->SetKeyword("ALBEDO_MAP", true);
KeywordMask mask = mat->GetKeywordMask();
```

---

## 3. 渲染 Pass 详解

### 3.1 Shadow Pass（级联阴影映射 CSM）

- 最大 4 级联，每级联 2048×2048 深度贴图
- **Practical Split Scheme** 混合对数/均匀分割（λ=0.95）
- 纹素对齐消除阴影边缘抖动
- PCF 软阴影：16 采样 Poisson Disk + 伪随机旋转核
- 深度纹理：`GL_DEPTH_COMPONENT32` + `GL_COMPARE_REF_TO_TEXTURE`（硬件 PCF）

### 3.2 Geometry Pass

- MSAA RGBA16F 渲染目标
- PBR 金属/粗糙度工作流
- IBL 环境光照（辐照度 + 预过滤辐射度 + BRDF LUT）
- 选中的 Entity 描边渲染（Stencil Buffer）
- 碰撞体线框可视化（编辑器模式）
- 全局网格（编辑器模式）

### 3.3 Bloom Pass

- 双 Pass 高斯模糊（水平 + 垂直分离）
- Bloom Blend Pass 混合到场景

### 3.4 Composite Pass

- MSAA Resolve（`glBlitFramebuffer`）→ RGBA8 最终输出
- 输出到 `m_CompositePass` RenderPass

---

## 4. Uniform Buffer 布局

### Per-Frame UBO（Binding 0）

由 `PrismShaderCompiler` 的 `PrismFrame.glsl` 定义，包含：
- `Prism_ViewProjection` / `Prism_InverseViewProjection` / `Prism_View` / `Prism_Projection`
- `Prism_Time` / `Prism_CameraPosition` / `Prism_DeltaTime`
- `Prism_Resolution` / `Prism_AspectRatio`
- `Prism_Lights[1]`：方向光（Direction + Radiance + Multiplier）
- `Prism_ShadowMatrices[4]` / `Prism_CascadeSplits` / `Prism_ShadowParams`

### Per-Object UBO（Binding 1）

- `Prism_Model` / `Prism_PreviousModel`
- `Prism_Bones[128]`（骨骼动画）

### PrismMaterial UBO（Binding 2）

- Properties 块中的非纹理变量自动打包到此 UBO
- 布局由 `CompiledShader::MaterialLayout` 确定

### 阴影贴图（Binding 3~6）

- 4 级联 `sampler2DShadow`：`Prism_ShadowMap0` ~ `Prism_ShadowMap3`
- 内置 `Prism_GetShadow(worldPos, viewDepth)` 采样函数

---

## 5. 编辑器控制

在 Editor 面板中提供：
- **阴影开关** + Shadow Bias / Normal Bias / Cascade 数量（1~4）调节
- **Bloom 开关** + 强度调节
- **网格开关** + 包围盒可视化开关
- **PBR 材质参数**实时调节
- **Shader 关键字** Checkbox
- **3D 物理设置**（重力、物理层）

---

## 6. 注意事项

1. 所有渲染资源使用 `Ref<T>` 侵入式智能指针管理
2. 阴影 FBO 使用 `GL_CLAMP_TO_EDGE`，超出 [0,1] 范围不产生边缘泄漏
3. CSM 对所有 Mesh 生效，暂不支持单独的投射/接收阴影标志
4. Geometry Pass 输出到 MSAA 纹理，Composite Pass 做 resolve
5. `RenderPipeline` 当前是单线程执行，多线程化在 [roadmap-render-thread.md](../.claude/roadmap-render-thread.md) 中规划
