#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Prism/Renderer/RenderPass.h"
#include "Prism/Renderer/RenderConfig.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Camera/Camera.h"
#include "Prism/Renderer/Buffer/FrameUniformBuffer.h"
#include "Prism/Renderer/Buffer/ObjectUniformBuffer.h"

namespace Prism
{
    class PrismShader;

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
    };

    struct FrameData
    {
        RendererCamera Camera;
        std::vector<DrawCommand> DrawList;
        std::vector<DrawCommand> SelectedDrawList;
        std::vector<DrawCommand> DebugDrawList;
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

        void Execute(const RenderConfig& config, const FrameData& data);

        Ref<RenderPass> GetFinalRenderPass() const { return m_CompositePass; }
        RenderPipelineOptions& GetOptions() { return m_Options; }

        static std::pair<Ref<TextureCube>, Ref<TextureCube>>
            CreateEnvironmentMap(const std::string& filepath);

    private:
        void BeginFrame(const RenderConfig& config, const FrameData& data);
        void UpdateShadowData(const RenderConfig& config, const FrameData& data);
        void ShadowPass(const std::vector<DrawCommand>& drawList);
        void GeometryPass(const RenderConfig config,
            const std::vector<DrawCommand>& drawList,
            const std::vector<DrawCommand>& selectedList,
            const std::vector<DrawCommand>& debugList);
        void CompositePass();

        static constexpr uint32_t SHADOW_MAP_SIZE = 2048;

        FrameUniformBuffer m_FrameUBO;
        ObjectUniformBuffer m_ObjectUBO;

        Ref<RenderPass> m_GeoPass;
        Ref<RenderPass> m_CompositePass;

        Ref<Material> m_CompositeMaterial;
        Ref<Material> m_GridMaterial;
        Ref<Material> m_OutlineMaterial;
        Ref<Material> m_ColliderMaterial;
        Ref<Material> m_ShadowDepthMaterial;

        Ref<Texture2D> m_BRDFLUT;
        Ref<PrismShader> m_CompositeShader;

        Ref<Framebuffer> m_ShadowFBOs[4];
        glm::mat4 m_ShadowMatrices[4]{};
        glm::vec4 m_CascadeSplits{};

        RenderPipelineOptions m_Options;
    };
}
