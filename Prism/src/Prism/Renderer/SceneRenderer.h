#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Prism/Renderer/Camera/Camera.h"
#include "Prism/Renderer/RenderConfig.h"

namespace Prism
{
    class PrismShader;
    class Material;
    class Mesh;
    class RenderPass;
    class Texture2D;
    class VertexBuffer;
    class IndexBuffer;
    class VertexInput;
    class Image2D;
    class UniformBuffer;

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

    struct SceneRendererOptions
    {
        bool ShowGrid = true;
        bool ShowBoundingBoxes = false;
    };

    class PRISM_API SceneRenderer : RefCounted
    {
    public:
        static SceneRenderer& Get();

        SceneRenderer();
        ~SceneRenderer();

        void Initialize(uint32_t viewportWidth, uint32_t viewportHeight);
        void Shutdown();
        void OnImGuiRender();
        void RT_Resize(uint32_t width, uint32_t height);

        void Execute(const FrameSnapshot& snapshot);

        SceneRendererOptions& GetOptions() { return m_Options; }
        Ref<Image2D> GetFinalImage() const;

    private:
        void ExecuteImpt(const FrameSnapshot& snapshot);

        void BeginFrame(const FrameSnapshot& snapshot);
        void UpdateShadowData(const FrameSnapshot& snapshot);
        void ShadowPass(const std::vector<DrawCommand>& drawList);
        void GeometryPass(const RenderConfig& config,
            const std::vector<DrawCommand>& drawList,
            const std::vector<DrawCommand>& selectedList,
            const std::vector<DrawCommand>& debugList);
        void IDPass(const std::vector<DrawCommand>& selectedList);
        void CompositePass();
        void BloomBlurPass();
        void BloomBlendPass();

        void DrawFullscreen(const Ref<Material>& material);
        void DrawQuad(const Ref<Material>& material, const glm::mat4& transform);
        void CreateFullscreenQuad();

        static constexpr uint32_t SHADOW_MAP_SIZE = 2048;

        static constexpr uint32_t PRISM_MAX_LIGHTS = 1;
        static constexpr uint32_t PRISM_MAX_CASCADES = 4;
        static constexpr uint32_t PRISM_MAX_BONES = 128;

        struct alignas(16) FrameData
        {
            glm::mat4 ViewProjection{ 1.0f };
            glm::mat4 InverseViewProjection{ 1.0f };
            glm::mat4 View{ 1.0f };
            glm::mat4 Projection{ 1.0f };

            glm::vec4 Time{ 0.0f };

            glm::vec3 CameraPosition{ 0.0f };
            float DeltaTime{ 0.0f };

            glm::vec2 Resolution{ 1280.0f, 720.0f };
            float AspectRatio{ 1.0f };
            float pad0{ 0.0f };

            struct Light
            {
                glm::vec3 Direction{};
                float pad1{};
                glm::vec3 Radiance{};
                float Multiplier{};
            } Lights[PRISM_MAX_LIGHTS];

            glm::mat4 ShadowMatrices[PRISM_MAX_CASCADES]{};
            glm::vec4 CascadeSplits{};
            glm::vec4 ShadowParams{};
            glm::vec4 ShadowData{};
        };

        struct alignas(16) ObjectData
        {
            glm::mat4 Model{ 1.0f };
            glm::mat4 PrevModel{ 1.0f };
            glm::vec4 Reserved{ 0.0f };
            glm::mat4 Bones[PRISM_MAX_BONES]{};
        };

        FrameData m_FrameData;
        Ref<UniformBuffer> m_FrameUBO;
        ObjectData m_ObjectData;
        Ref<UniformBuffer> m_ObjectUBO;
        bool m_ObjectBonesDirty = false;

        void SetObjectBones(const glm::mat4* bones, uint32_t count);
        void UploadObjectUBO();

        Ref<VertexInput> m_FullscreenQuadPipeline;

        Ref<RenderPass> m_GeoPass;
        Ref<RenderPass> m_IDPass;
        Ref<RenderPass> m_CompositePass;
        Ref<RenderPass> m_BloomBlurPass[2];
        Ref<RenderPass> m_BloomBlendPass;

        Ref<Material> m_CompositeMaterial;
        Ref<Material> m_BloomBlurMaterial;
        Ref<Material> m_BloomBlendMaterial;
        Ref<Material> m_GridMaterial;
        Ref<Material> m_IDMaterial;
        Ref<Material> m_IDAnimMaterial;
        Ref<Material> m_ColliderMaterial;

        Ref<Texture2D> m_BRDFLUT;

        Ref<RenderPass> m_ShadowPasses[4];
        glm::mat4 m_ShadowMatrices[4]{};
        glm::vec4 m_CascadeSplits{};

        SceneRendererOptions m_Options;

        static Ref<SceneRenderer> s_Instance;
    };
}
