#pragma once

#include "Prism/Core/Ref.h"
#include "Prism/Renderer/RendererTypes.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"
#include <PrismShaderCore/Pipeline/PipelineState.h>

#include "Platform/Vulkan/Vulkan.h"

#include <map>
#include <mutex>
#include <span>
#include <unordered_map>

namespace Prism
{
    class VulkanShader;
    class VulkanFramebuffer;
    class VertexBufferLayout;

    struct VulkanPipelineSpecification
    {
        WeakRef<VulkanShader> Shader;
        std::span<const VertexBufferLayout> VertexLayouts;
        PrismShaderCompiler::PipelineState State;
        WeakRef<VulkanFramebuffer> Framebuffer;
        PrimitiveType Topology = PrimitiveType::Triangles;
    };

    class VulkanPipeline : public RefCounted
    {
    public:
        VulkanPipeline(const VulkanPipelineSpecification& spec, VkPipelineCache pipelineCache);
        virtual ~VulkanPipeline();

        void RT_Bind(VkCommandBuffer cmdBuf) const;
        void RT_BindGlobalSet(VkCommandBuffer cmdBuf, VkDescriptorSet set) const;
        void RT_BindRenderPassSet(VkCommandBuffer cmdBuf, VkDescriptorSet set) const;
        void RT_BindMaterialSet(VkCommandBuffer cmdBuf, VkDescriptorSet set) const;
        void RT_BindPushConstant(VkCommandBuffer cmdBuf, uint32_t offset, uint32_t size, const void* data) const;
    private:
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

        mutable VkCommandBuffer m_BoundCommandBuffer = VK_NULL_HANDLE;
        mutable VkDescriptorSet m_GlobalSet = VK_NULL_HANDLE;
        mutable VkDescriptorSet m_PassSet = VK_NULL_HANDLE;
        mutable VkDescriptorSet m_MaterialSet = VK_NULL_HANDLE;
    };

    class VulkanComputePipeline : public RefCounted
    {
    public:
        VulkanComputePipeline(WeakRef<VulkanShader> shader, VkPipelineCache pipelineCache);
        virtual ~VulkanComputePipeline();

        void RT_Execute(VkDescriptorSet* sets, uint32_t setCount, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    private:
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        mutable VkCommandBuffer m_BoundCommandBuffer = VK_NULL_HANDLE;
    };

    class VulkanPipelineCache
    {
    public:
        void Init();
        void Shutdown();

        WeakRef<VulkanPipeline> Get(const VulkanPipelineSpecification& spec);
        WeakRef<VulkanComputePipeline> GetCompute(VulkanShader* shader);

        void Erase(VulkanShader* shader);
    private:
        struct Entry
        {
            Ref<VulkanPipeline> Pipeline;
            const VulkanShader* Shader = nullptr;
        };

        std::mutex m_Mutex;
        std::unordered_map<uint64_t, Entry> m_Pipelines;
        std::unordered_map<VulkanShader*, Ref<VulkanComputePipeline>> m_ComputePipelines;
        VkPipelineCache m_VkPipelineCache = VK_NULL_HANDLE;
    };
}
