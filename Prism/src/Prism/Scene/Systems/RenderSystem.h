#pragma once

#include "ISystem.h"
#include "Prism/Renderer/RenderPipeline.h"

namespace Prism
{
    class EditorCamera;

    class PRISM_API RenderSystem : public ISystem
    {
    public:
        explicit RenderSystem(Scene* scene);
        ~RenderSystem() override;

        void OnCreate() override;
        void OnDestroy() override;
        void OnPostLateUpdate(float dt) override;
        void Render();

        void SetEditorCamera(const EditorCamera& camera);
        void SetViewportSize(uint32_t width, uint32_t height);

        void SubmitDebugMesh(Ref<Mesh> mesh, const glm::mat4& transform,
                             Ref<Material> material = nullptr);

        Ref<RenderPass> GetFinalRenderPass();
        uint32_t GetFinalColorBufferID();
        RenderPipelineOptions& GetOptions();
        RenderConfig& GetConfig() { return m_Config; }
        const RenderConfig& GetConfig() const { return m_Config; }

        static std::pair<Ref<TextureCube>, Ref<TextureCube>>
            CreateEnvironmentMap(const std::string& filepath);

    private:
        Scene* m_Scene;
        std::unique_ptr<RenderPipeline> m_Pipeline;
        RenderConfig m_Config;
        uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

        bool m_HasEditorCamera = false;
        RendererCamera m_EditorCamera;
        FrameData m_PendingFrameData;

        void CollectMeshRenderers(FrameData& data);
        void CollectDebugDraws(FrameData& data);
    };
}
