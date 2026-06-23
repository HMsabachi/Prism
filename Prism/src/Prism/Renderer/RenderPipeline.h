#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Prism/Renderer/Camera/Camera.h"
#include "Prism/Renderer/RenderConfig.h"
#include "Prism/Renderer/Buffer/FrameUniformBuffer.h"
#include "Prism/Renderer/Buffer/ObjectUniformBuffer.h"

namespace Prism
{
    class PrismShader;
    class Material;
    class Mesh;
    class RenderPass;
    class Texture2D;
    class VertexBuffer;
    class IndexBuffer;
    class Pipeline;

    struct RendererCamera
    {
        Camera Projection;
        glm::mat4 ViewMatrix = glm::mat4(1.0f);
    };

    struct DrawCommand
    {
        Ref<Mesh> Mesh;
        uint32_t SubmeshIndex = 0;
        Ref<Material> Material;
        glm::mat4 Transform = glm::mat4(1.0f);
        uint64_t SortKey = 0;
    };

    struct FrameSnapshot
    {
        RendererCamera Camera;
        RenderConfig Config;
        std::vector<DrawCommand> DrawList;
        std::vector<DrawCommand> SelectedDrawList;
        std::vector<DrawCommand> ShadowDrawList;
        std::vector<DrawCommand> DebugDrawList;
        uint64_t FrameIndex = 0;
    };

    struct RenderPipelineOptions
    {
        bool ShowGrid = true;
        bool ShowBoundingBoxes = false;
    };

    class PRISM_API RenderPipeline
    {
    public:
        void Initialize(uint32_t viewportWidth, uint32_t viewportHeight);
        void Shutdown();
        void Resize(uint32_t width, uint32_t height);

        void Execute(const FrameSnapshot& snapshot);

        Ref<RenderPass> GetFinalRenderPass() const { return m_CompositePass; }
        const Ref<RenderPass>& GetShadowPass(uint32_t index) const { return m_ShadowPasses[index]; }
        RenderPipelineOptions& GetOptions() { return m_Options; }

        static std::pair<Ref<TextureCube>, Ref<TextureCube>>
            CreateEnvironmentMap(const std::string& filepath);

    private:
        void BeginFrame(const FrameSnapshot& snapshot);
        void UpdateShadowData(const FrameSnapshot& snapshot);
        void ShadowPass(const std::vector<DrawCommand>& drawList);
        void GeometryPass(const RenderConfig& config,
            const std::vector<DrawCommand>& drawList,
            const std::vector<DrawCommand>& selectedList,
            const std::vector<DrawCommand>& debugList);
        void CompositePass();
        void BloomBlurPass();
        void BloomBlendPass();

        void DrawFullscreen(const Ref<Material>& material);
        void DrawQuad(const Ref<Material>& material, const glm::mat4& transform);
        void CreateFullscreenQuad();

        static constexpr uint32_t SHADOW_MAP_SIZE = 2048;

        FrameUniformBuffer m_FrameUBO;
        ObjectUniformBuffer m_ObjectUBO;

        Ref<VertexBuffer> m_FullscreenQuadVB;
        Ref<IndexBuffer> m_FullscreenQuadIB;
        Ref<Pipeline> m_FullscreenQuadPipeline;

        Ref<RenderPass> m_GeoPass;
        Ref<RenderPass> m_CompositePass;
        Ref<RenderPass> m_BloomBlurPass[2];
        Ref<RenderPass> m_BloomBlendPass;

        Ref<Material> m_CompositeMaterial;
        Ref<Material> m_BloomBlurMaterial;
        Ref<Material> m_BloomBlendMaterial;
        Ref<Material> m_GridMaterial;
        Ref<Material> m_OutlineMaterial;
        Ref<Material> m_ColliderMaterial;
        Ref<Material> m_ShadowDepthMaterial;

        Ref<Texture2D> m_BRDFLUT;
        Ref<PrismShader> m_CompositeShader;

        Ref<RenderPass> m_ShadowPasses[4];
        glm::mat4 m_ShadowMatrices[4]{};
        glm::vec4 m_CascadeSplits{};

        RenderPipelineOptions m_Options;
    };
}
