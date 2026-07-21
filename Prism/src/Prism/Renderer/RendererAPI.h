#pragma once
#include "RendererTypes.h"
#include "Prism/Utilities/BitFlags.h"
#include "Prism/Core/Ref.h"

#include <string>
#include <utility>
#include <glm/glm.hpp>

namespace PrismShaderCompiler { struct PipelineState; }

namespace Prism
{
    class RenderPass;
    class VertexInput;
    class Mesh;
    class Material;
    class TextureCube;
    class Image2D;
    class SceneEnvironment;

    using RendererID = uint32_t;

    enum class RendererAPIType
    {
        None = 0,
        OpenGL = 1
    };

    struct RenderAPICapabilities
    {
        std::string Vendor;
        std::string Renderer;
        std::string Version;

        int MaxSamples = 0;
        float MaxAnisotropy = 0.0f;
        int MaxTextureUnits = 0;

        int MaxGroupCount[3]{}, MaxGroupSize[3]{}, MaxInvocations{};
    };

    class PRISM_API RendererAPI
    {
    public:
#pragma region 数据结构
        enum class PRISM_API Barrier
        {
            None = 0,
            ShaderStorage = BIT(0),
            VertexAttribArray = BIT(1),
            ElementArray = BIT(2),
            ImageAccess = BIT(3),
            TextureFetch = BIT(4),
            TextureUpdate = BIT(5),
            Framebuffer = BIT(6),
            Command = BIT(7),
            All = BIT(8)
        };
        typedef BitFlags<Barrier> BarrierFlags;
#pragma endregion

    public:
        virtual ~RendererAPI() = default;

        // 高层纯虚接口（对齐 Hazel，后端多态分发）
        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true) = 0;
        virtual void EndRenderPass() = 0;
        virtual void SubmitFullscreenQuad(Ref<VertexInput> vertexInput, Ref<Material> material,
            const PrismShaderCompiler::PipelineState* stateOverride = nullptr) = 0;

        virtual void SetSceneEnvironment(const Ref<SceneEnvironment>& environment, const Ref<Image2D>& shadow) = 0;
        virtual std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath) = 0;

        // RenderMesh 适配 Prism：从 DrawCommand 取 material + 单 submeshIndex + PSL pass（决策6，不引入 RenderMeshWithoutMaterial）
        // stateOverride：可选 RenderState 覆盖（Unity RenderStateBlock 模式），非空时其标记字段 Merge 进 PSO effectiveState（描边 write pass 用）。
        virtual void RenderMesh(Ref<VertexInput> vertexInput, Ref<Mesh> mesh, Ref<Material> material,
            uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass,
            const PrismShaderCompiler::PipelineState* stateOverride = nullptr) = 0;
        virtual void RenderQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const glm::mat4& transform,
            const PrismShaderCompiler::PipelineState* stateOverride = nullptr) = 0;

        virtual RenderAPICapabilities& GetCapabilities() = 0;

        static RendererAPIType Current() { return s_CurrentRendererAPI; }
    private:
        static RendererAPIType s_CurrentRendererAPI;
    };
    using MBarrier = RendererAPI::Barrier;
}
