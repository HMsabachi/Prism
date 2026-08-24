#pragma once

#include "Prism/Core/Ref.h"
#include "Prism/Renderer/RendererTypes.h"
#include <PrismShaderCore/Pipeline/PipelineState.h>

#include "Platform/Vulkan/Vulkan.h"

#include "Prism/Utilities/StaticVector.h"
#include <span>
#include <vector>

namespace Prism
{
    class VulkanShader;
    class VulkanFramebuffer;
    class VertexBufferLayout;

    struct VulkanPSOKey
    {
        VulkanShader* Shader = nullptr;
        uint64_t VertexLayoutHash = 0;
        PrismShaderCompiler::PipelineState State;
        StaticVector<VkFormat, 10> AttachmentFormats;
        uint32_t ColorAttachmentCount = 0;
        uint32_t Samples = 1;
        PrimitiveType Topology = PrimitiveType::Triangles;

        bool operator==(const VulkanPSOKey& other) const;
    };

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
        VulkanPipeline(const VulkanPSOKey& key, const Ref<VulkanShader>& shader,
            std::span<const VertexBufferLayout> vertexLayouts, const Ref<VulkanFramebuffer>& framebuffer);
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
        static Ref<VulkanPipeline> Get(const Ref<VulkanShader>& shader,
            std::span<const VertexBufferLayout> vertexLayouts,
            const PrismShaderCompiler::PipelineState& state,
            const Ref<VulkanFramebuffer>& framebuffer,
            PrimitiveType topology = PrimitiveType::Triangles);
        static void Erase(VulkanShader* shader);
        static void Clear();
    private:
        static Ref<VulkanPipeline> Create(const VulkanPSOKey& key, const Ref<VulkanShader>& shader,
            std::span<const VertexBufferLayout> vertexLayouts, const Ref<VulkanFramebuffer>& framebuffer);
    };
}

namespace std
{
    template<>
    struct hash<Prism::VulkanPSOKey>
    {
        size_t operator()(const Prism::VulkanPSOKey& key) const noexcept;
    };
}
