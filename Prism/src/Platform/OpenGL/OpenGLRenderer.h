#pragma once

#include "Prism/Renderer/RendererAPI.h"

namespace Prism
{
    // OpenGL 后端高层渲染器（实现 RendererAPI 高层纯虚接口）。
    // 不 include glad：GL 调用全部下沉 .cpp，保持后端 .h 与 Vulkan 后端对称（Vulkan 框架预留）。
    class OpenGLRenderer : public RendererAPI
    {
    public:
        virtual void Init() override;
        virtual void Shutdown() override;

        virtual void BeginFrame() override;
        virtual void EndFrame() override;

        // 完整实现（RenderPipeline/EditorLayer 现有调用点依赖，Phase 4 不能断渲染）
        virtual void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true) override;
        virtual void EndRenderPass() override;

        // 占位（Phase 5 接入 RenderPipeline 时实现）
        virtual void SubmitFullscreenQuad(Ref<Pipeline> pipeline, Ref<Material> material) override;

        virtual void SetSceneEnvironment(const Ref<SceneEnvironment>& environment, const Ref<Image2D>& shadow) override;
        virtual std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath) override;

        virtual void RenderMesh(Ref<Pipeline> pipeline, Ref<Mesh> mesh, Ref<Material> material,
            uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass) override;
        virtual void RenderQuad(Ref<Pipeline> pipeline, Ref<Material> material, const glm::mat4& transform) override;

        virtual RenderAPICapabilities& GetCapabilities() override;
    };
}
