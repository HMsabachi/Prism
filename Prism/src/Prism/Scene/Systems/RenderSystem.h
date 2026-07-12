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
        void OnRender(float dt) override;
        void OnImGuiRender() override;

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
        // TODO(多线程): 单缓冲→双缓冲 m_Snapshots[2] + m_FrameIndex 交替,逻辑写 N+1/渲染读 N。详见 .claude/roadmap-render-thread.md Phase 3
        FrameSnapshot m_PendingSnapshot;

        void Render();
        void CollectMeshRenderers(FrameSnapshot& snapshot);
        void CollectDebugDraws(FrameSnapshot& snapshot);
    };
}
