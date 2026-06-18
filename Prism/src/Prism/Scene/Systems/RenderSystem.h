#pragma once

#include "ISystem.h"
#include "Prism/Renderer/RenderPipeline.h"

namespace Prism
{
    class EditorCamera;

    class RenderSystem : public ISystem
    {
    public:
        explicit RenderSystem(Scene* scene);
        ~RenderSystem() override;

        void OnCreate() override;
        void OnDestroy() override;
        void OnPostLateUpdate(float dt) override;

        void SetEditorCamera(const EditorCamera& camera);
        void SetViewportSize(uint32_t width, uint32_t height);

        void SubmitDebugMesh(Ref<Mesh> mesh, const glm::mat4& transform,
                             Ref<Material> material = nullptr);

        Ref<RenderPass> GetFinalRenderPass();
        RenderConfig& GetConfig() { return m_Config; }
        const RenderConfig& GetConfig() const { return m_Config; }

        static std::pair<Ref<TextureCube>, Ref<TextureCube>>
            CreateEnvironmentMap(const std::string& filepath);

    private:
        Scene* m_Scene;
        std::unique_ptr<RenderPipeline> m_Pipeline;
        RenderConfig m_Config;

        bool m_HasEditorCamera = false;
        RendererCamera m_EditorCamera;
        FrameData m_PendingFrameData;

        void CollectMeshRenderers(FrameData& data);
        void CollectDebugDraws(FrameData& data);
    };
}
