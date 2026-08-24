#pragma once

#include "Prism/Core/Ref.h"
#include "Prism/Renderer/RendererTypes.h"
#include <PrismShaderCore/Pipeline/PipelineState.h>

#include "Platform/Vulkan/Vulkan.h"

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
        Ref<VulkanShader> Shader;
        std::span<const VertexBufferLayout> VertexLayouts;
        PrismShaderCompiler::PipelineState State;
        Ref<VulkanFramebuffer> Framebuffer;
        PrimitiveType Topology = PrimitiveType::Triangles;
    };

    class VulkanPipeline : public RefCounted
    {
    public:
        VulkanPipeline(const VulkanPipelineSpecification& spec, VkPipelineCache pipelineCache);
        virtual ~VulkanPipeline();

        void RT_Bind() const;
    private:
        friend class VulkanPipelineCache;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    };

    class VulkanPipelineCache
    {
    public:
        void Init();
        void Shutdown();

        Ref<VulkanPipeline> Get(const VulkanPipelineSpecification& spec);
        void Erase(VulkanShader* shader);
    private:
        struct Entry
        {
            Ref<VulkanPipeline> Pipeline;
            const VulkanShader* Shader = nullptr;
        };

        Ref<VulkanPipeline> Create(const VulkanPipelineSpecification& spec);

        std::mutex m_Mutex;
        std::unordered_map<uint64_t, Entry> m_Pipelines;
        VkPipelineCache m_VkPipelineCache = VK_NULL_HANDLE;
    };
}
