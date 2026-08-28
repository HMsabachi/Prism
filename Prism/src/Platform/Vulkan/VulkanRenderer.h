#pragma once

#include "Prism/Renderer/RendererAPI.h"
#include "Platform/Vulkan/Vulkan.h"

namespace Prism
{
    class VulkanPipelineCache;
    class VulkanTexture2D;
    class VulkanTextureCube;
    class VulkanImage2D;
    class VulkanImageCube;
    class VulkanUniformBuffer;
    class VulkanShaderStorageBuffer;

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

        virtual void SetGlobalUniformBuffer(uint32_t binding, Ref<UniformBuffer> ubo) override;
        virtual void SetGlobalShaderStorageBuffer(uint32_t binding, Ref<ShaderStorageBuffer> ssbo) override;
        virtual void SetGlobalTexture(uint32_t binding, Ref<Image> image) override;
        virtual void BakeGlobalInputs() override;

        virtual void RenderMesh(Ref<Mesh> mesh, uint32_t submeshIndex, Ref<Material> material,
            uint32_t passIndex, uint32_t drawIndex = 0) override;
        virtual void RenderQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex = 0) override;

        virtual void DispatchCompute(Ref<ComputeShader> computeShader, int32_t kernel,
            uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) override;

        virtual RenderAPICapabilities& GetCapabilities() override;

        virtual void OnImGuiRender() override;

        static VulkanPipelineCache& GetPipelineCache();

        static WeakRef<VulkanImage2D> RT_GetBlackImage2D();
        static WeakRef<VulkanImageCube> RT_GetBlackImageCube();
        static WeakRef<VulkanUniformBuffer> RT_GetEmptyUniformBuffer();
        static WeakRef<VulkanShaderStorageBuffer> RT_GetEmptyShaderStorageBuffer();
    };

}
