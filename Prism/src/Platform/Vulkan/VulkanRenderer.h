#pragma once

#include "Prism/Renderer/RendererAPI.h"
#include "Platform/Vulkan/Vulkan.h"

namespace Prism
{

    class PRISM_API VulkanRenderer : public RendererAPI
    {
    public:
        virtual void Init() override;
        virtual void Shutdown() override;

        virtual void BeginFrame() override;
        virtual void EndFrame() override;

        virtual void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true) override;
        virtual void EndRenderPass() override;
        virtual void SubmitFullscreenQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex = 0) override;

        virtual void SetSceneEnvironment(const Ref<SceneEnvironment>& environment) override;
        virtual std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath) override;

        virtual void SetUniformBuffer(uint32_t set, uint32_t binding, Ref<UniformBuffer> ubo) override;
        virtual void SetShaderStorageBuffer(uint32_t set, uint32_t binding, Ref<ShaderStorageBuffer> ssbo) override;
        virtual void SetTexture(uint32_t set, uint32_t binding, Ref<Image> image) override;

        virtual void RenderMesh(Ref<Mesh> mesh, uint32_t submeshIndex, Ref<Material> material,
            uint32_t passIndex, uint32_t drawIndex = 0) override;
        virtual void RenderQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex = 0) override;

        virtual void DispatchCompute(Ref<Shader> kernelShader,
            const std::vector<ComputeResourceBinding>& bindings,
            uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) override;

        virtual RenderAPICapabilities& GetCapabilities() override;

        static VkCommandBuffer GetCurrentCommandBuffer();
    };

}
