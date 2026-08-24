#include "prpch.h"
#include "VulkanPipeline.h"

#include "VulkanContext.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderer.h"
#include "VulkanShader.h"

#include "Prism/Renderer/Buffer/VertexBuffer.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/ShaderCompiler/PrismBindings.h"
#include "Prism/Utilities/StaticVector.h"

namespace Prism
{
    namespace Utils
    {
        inline static uint64_t CombineLayoutHash(std::span<const VertexBufferLayout> layouts)
        {
            constexpr uint64_t FNV_PRIME = 1099511628211ULL;
            constexpr uint64_t OFFSET_BASIS = 14695981039346656037ULL;
            uint64_t hash = OFFSET_BASIS;
            for (const auto& layout : layouts)
            {
                hash ^= layout.GetHash();
                hash *= FNV_PRIME;
            }
            return hash;
        }

        inline static uint64_t CalculatePipelineSpecificationHash(const VulkanPipelineSpecification& spec)
        {
            constexpr uint64_t FNV_PRIME = 1099511628211ULL;
            constexpr uint64_t OFFSET_BASIS = 14695981039346656037ULL;
            uint64_t hash = OFFSET_BASIS;
            hash ^= reinterpret_cast<uint64_t>(spec.Shader.Raw()); hash *= FNV_PRIME;
            hash ^= CombineLayoutHash(spec.VertexLayouts); hash *= FNV_PRIME;
            hash ^= spec.Framebuffer->GetHash(); hash *= FNV_PRIME;
            hash ^= spec.State.Hash; hash *= FNV_PRIME;
            return hash;
        }

        inline static VkFormat ShaderDataTypeToVulkanFormat(ShaderDataType type)
        {
            switch (type)
            {
            case ShaderDataType::Float:   return VK_FORMAT_R32_SFLOAT;
            case ShaderDataType::Float2:  return VK_FORMAT_R32G32_SFLOAT;
            case ShaderDataType::Float3:  return VK_FORMAT_R32G32B32_SFLOAT;
            case ShaderDataType::Float4:  return VK_FORMAT_R32G32B32A32_SFLOAT;
            case ShaderDataType::Int:     return VK_FORMAT_R32_SINT;
            case ShaderDataType::Int2:    return VK_FORMAT_R32G32_SINT;
            case ShaderDataType::Int3:    return VK_FORMAT_R32G32B32_SINT;
            case ShaderDataType::Int4:    return VK_FORMAT_R32G32B32A32_SINT;
            case ShaderDataType::Bool:    return VK_FORMAT_R32_UINT;
            }
            PR_CORE_ASSERT(false, "ShaderDataTypeToVulkanFormat: Unsupported vertex attribute type!");
            return VK_FORMAT_UNDEFINED;
        }

        inline static VkPrimitiveTopology ToVulkan(PrimitiveType topology)
        {
            switch (topology)
            {
            case PrimitiveType::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case PrimitiveType::Lines:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            }
            PR_CORE_ASSERT(false, "ToVulkan: Unsupported primitive topology!");
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }

        inline static VkCullModeFlags ToVulkan(PrismShaderCompiler::CullMode mode)
        {
            switch (mode)
            {
            case PrismShaderCompiler::CullMode::Off:   return VK_CULL_MODE_NONE;
            case PrismShaderCompiler::CullMode::Back:  return VK_CULL_MODE_BACK_BIT;
            case PrismShaderCompiler::CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
            }
            return VK_CULL_MODE_BACK_BIT;
        }

        inline static VkCompareOp ToVulkan(PrismShaderCompiler::DepthFunc func)
        {
            switch (func)
            {
            case PrismShaderCompiler::DepthFunc::Never:    return VK_COMPARE_OP_NEVER;
            case PrismShaderCompiler::DepthFunc::Less:     return VK_COMPARE_OP_LESS;
            case PrismShaderCompiler::DepthFunc::Equal:    return VK_COMPARE_OP_EQUAL;
            case PrismShaderCompiler::DepthFunc::LEqual:   return VK_COMPARE_OP_LESS_OR_EQUAL;
            case PrismShaderCompiler::DepthFunc::Greater:  return VK_COMPARE_OP_GREATER;
            case PrismShaderCompiler::DepthFunc::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
            case PrismShaderCompiler::DepthFunc::GEqual:   return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case PrismShaderCompiler::DepthFunc::Always:   return VK_COMPARE_OP_ALWAYS;
            }
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        }

        inline static VkBlendFactor ToVulkan(PrismShaderCompiler::BlendFactor factor)
        {
            switch (factor)
            {
            case PrismShaderCompiler::BlendFactor::Zero:             return VK_BLEND_FACTOR_ZERO;
            case PrismShaderCompiler::BlendFactor::One:              return VK_BLEND_FACTOR_ONE;
            case PrismShaderCompiler::BlendFactor::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
            case PrismShaderCompiler::BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case PrismShaderCompiler::BlendFactor::DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
            case PrismShaderCompiler::BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            }
            return VK_BLEND_FACTOR_ONE;
        }

        inline static VkCompareOp ToVulkan(PrismShaderCompiler::StencilFunc func)
        {
            switch (func)
            {
            case PrismShaderCompiler::StencilFunc::Never:    return VK_COMPARE_OP_NEVER;
            case PrismShaderCompiler::StencilFunc::Less:     return VK_COMPARE_OP_LESS;
            case PrismShaderCompiler::StencilFunc::Equal:    return VK_COMPARE_OP_EQUAL;
            case PrismShaderCompiler::StencilFunc::LEqual:   return VK_COMPARE_OP_LESS_OR_EQUAL;
            case PrismShaderCompiler::StencilFunc::Greater:  return VK_COMPARE_OP_GREATER;
            case PrismShaderCompiler::StencilFunc::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
            case PrismShaderCompiler::StencilFunc::GEqual:   return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case PrismShaderCompiler::StencilFunc::Always:   return VK_COMPARE_OP_ALWAYS;
            }
            return VK_COMPARE_OP_ALWAYS;
        }

        inline static VkStencilOp ToVulkan(PrismShaderCompiler::StencilOp op)
        {
            switch (op)
            {
            case PrismShaderCompiler::StencilOp::Keep:     return VK_STENCIL_OP_KEEP;
            case PrismShaderCompiler::StencilOp::Zero:     return VK_STENCIL_OP_ZERO;
            case PrismShaderCompiler::StencilOp::Replace:  return VK_STENCIL_OP_REPLACE;
            case PrismShaderCompiler::StencilOp::Incr:     return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            case PrismShaderCompiler::StencilOp::IncrWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
            case PrismShaderCompiler::StencilOp::Decr:     return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            case PrismShaderCompiler::StencilOp::DecrWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
            case PrismShaderCompiler::StencilOp::Invert:   return VK_STENCIL_OP_INVERT;
            }
            return VK_STENCIL_OP_KEEP;
        }

        inline static VkPolygonMode ToVulkan(PrismShaderCompiler::PolygonMode mode)
        {
            switch (mode)
            {
            case PrismShaderCompiler::PolygonMode::Fill:  return VK_POLYGON_MODE_FILL;
            case PrismShaderCompiler::PolygonMode::Line:  return VK_POLYGON_MODE_LINE;
            case PrismShaderCompiler::PolygonMode::Point: return VK_POLYGON_MODE_POINT;
            }
            return VK_POLYGON_MODE_FILL;
        }

        inline static VkColorComponentFlags ToVulkan(PrismShaderCompiler::ColorMask mask)
        {
            switch (mask)
            {
            case PrismShaderCompiler::ColorMask::RGBA: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            case PrismShaderCompiler::ColorMask::RGB:  return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
            case PrismShaderCompiler::ColorMask::R:    return VK_COLOR_COMPONENT_R_BIT;
            case PrismShaderCompiler::ColorMask::G:    return VK_COLOR_COMPONENT_G_BIT;
            case PrismShaderCompiler::ColorMask::B:    return VK_COLOR_COMPONENT_B_BIT;
            case PrismShaderCompiler::ColorMask::A:    return VK_COLOR_COMPONENT_A_BIT;
            case PrismShaderCompiler::ColorMask::None: return 0;
            }
            return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }
    }

    VulkanPipeline::VulkanPipeline(const VulkanPipelineSpecification& spec, VkPipelineCache pipelineCache)
    {
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
        const auto& state = spec.State;

        // 顶点输入：每个 vertex buffer 一个 binding，location 与 PSL codegen 的 SemanticToLocation 对齐
        PR_CORE_ASSERT(!spec.VertexLayouts.empty(), "VulkanPipeline: VertexLayouts must not be empty");
        StaticVector<VkVertexInputBindingDescription, 8> bindingDescriptions;
        StaticVector<VkVertexInputAttributeDescription, 15> attributeDescriptions;
        size_t attributeCount = 0;
        for (const auto& layout : spec.VertexLayouts)
            attributeCount += layout.GetElements().size();
        for (size_t i = 0; i < spec.VertexLayouts.size(); i++)
        {
            const VertexBufferLayout& layout = spec.VertexLayouts[i];
            bindingDescriptions.emplace_back(VkVertexInputBindingDescription{
                static_cast<uint32_t>(i), layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX });
            for (const auto& element : layout.GetElements())
            {
                attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
                    PrismShaderCompiler::SemanticToLocation(element.Semantic), static_cast<uint32_t>(i),
                    Utils::ShaderDataTypeToVulkanFormat(element.Type), element.Offset });
            }
        }

        VkPipelineVertexInputStateCreateInfo vertexInputState{};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.topology = Utils::ToVulkan(spec.Topology);

        bool needDepthBias = state.DepthBiasFactor != 0.0f || state.DepthBiasUnits != 0.0f;
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.polygonMode = Utils::ToVulkan(state.FillMode);
        rasterizationState.cullMode = Utils::ToVulkan(state.Cull);
        rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationState.depthClampEnable = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.depthBiasEnable = needDepthBias ? VK_TRUE : VK_FALSE;
        rasterizationState.depthBiasConstantFactor = state.DepthBiasUnits;
        rasterizationState.depthBiasSlopeFactor = state.DepthBiasFactor;
        rasterizationState.lineWidth = state.LineWidth;

        // 每个 color attachment 一份 blend 状态（未启用混合也要 colorWriteMask）
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates(spec.Framebuffer->GetColorAttachmentCount());
        for (auto& blendAttachment : blendAttachmentStates)
        {
            blendAttachment.blendEnable = state.BlendEnabled ? VK_TRUE : VK_FALSE;
            blendAttachment.srcColorBlendFactor = Utils::ToVulkan(state.SrcFactor);
            blendAttachment.dstColorBlendFactor = Utils::ToVulkan(state.DstFactor);
            blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            blendAttachment.srcAlphaBlendFactor = Utils::ToVulkan(state.SrcAlpha);
            blendAttachment.dstAlphaBlendFactor = Utils::ToVulkan(state.DstAlpha);
            blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            blendAttachment.colorWriteMask = Utils::ToVulkan(state.WriteMask);
        }

        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
        colorBlendState.pAttachments = blendAttachmentStates.data();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable = state.DepthTest ? VK_TRUE : VK_FALSE;
        depthStencilState.depthWriteEnable = state.DepthWrite ? VK_TRUE : VK_FALSE;
        depthStencilState.depthCompareOp = Utils::ToVulkan(state.DepthCompare);
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.stencilTestEnable = state.StencilTest ? VK_TRUE : VK_FALSE;
        depthStencilState.back.failOp = Utils::ToVulkan(state.StencilFailOp);
        depthStencilState.back.passOp = Utils::ToVulkan(state.StencilPassOp);
        depthStencilState.back.depthFailOp = Utils::ToVulkan(state.StencilDepthFailOp);
        depthStencilState.back.compareOp = Utils::ToVulkan(state.StencilCompare);
        depthStencilState.back.compareMask = state.StencilReadMask;
        depthStencilState.back.writeMask = state.StencilWriteMask;
        depthStencilState.back.reference = (uint32_t)state.StencilRef;
        depthStencilState.front = depthStencilState.back;

        VkPipelineMultisampleStateCreateInfo multisampleState{};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.rasterizationSamples = (VkSampleCountFlagBits)spec.Framebuffer->GetSamples();
        multisampleState.pSampleMask = nullptr;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = spec.Shader->GetPipelineLayout();
        pipelineInfo.renderPass = spec.Framebuffer->GetRenderPass();
        pipelineInfo.stageCount = static_cast<uint32_t>(spec.Shader->GetPipelineShaderStageCreateInfos().size());
        pipelineInfo.pStages = spec.Shader->GetPipelineShaderStageCreateInfos().data();
        pipelineInfo.pVertexInputState = &vertexInputState;
        pipelineInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineInfo.pRasterizationState = &rasterizationState;
        pipelineInfo.pColorBlendState = &colorBlendState;
        pipelineInfo.pMultisampleState = &multisampleState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pDynamicState = &dynamicState;

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1,
            &pipelineInfo, nullptr, &m_Pipeline));
        m_PipelineLayout = spec.Shader->GetPipelineLayout();
    }

    VulkanPipeline::~VulkanPipeline()
    {
        if (!m_Pipeline)
            return;

        VkPipeline pipeline = m_Pipeline;
        Renderer::SubmitResourceFree([pipeline]()
        {
            auto device = VulkanContext::GetCurrentDevice();
            PR_CORE_ASSERT(device, "VulkanPipeline::~VulkanPipeline: VulkanContext::GetCurrentDevice() returned nullptr");
            vkDestroyPipeline(device->GetVulkanDevice(), pipeline, nullptr);
        });
    }


    void VulkanPipeline::RT_Bind(VkCommandBuffer cmdBuf) const
    {
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }
    void VulkanPipeline::RT_BindGolbalSet(VkCommandBuffer cmdBuf, VkDescriptorSet set) const
    {
        if (set == m_GlobalSet || set == VK_NULL_HANDLE) return;
        vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout, Config::PRISM_VULKAN_SET_GLOBAL, 1, &set, 0, nullptr);
    }
    void VulkanPipeline::RT_BindRenderPassSet(VkCommandBuffer cmdBuf, VkDescriptorSet set) const
    {
        if (set == m_PassSet || set == VK_NULL_HANDLE) return;
        vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout, Config::PRISM_VULKAN_SET_RENDER_PASS, 1, &set, 0, nullptr);
    }
    void VulkanPipeline::RT_BindMaterialSet(VkCommandBuffer cmdBuf, VkDescriptorSet set) const
    {
        if (set == m_MaterialSet || set == VK_NULL_HANDLE) return;
        vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout, Config::PRISM_VULKAN_SET_MATERIAL, 1, &set, 0, nullptr);
    }

    void VulkanPipelineCache::Init()
    {
        // TODO: 磁盘持久化
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreatePipelineCache(VulkanContext::GetCurrentDevice()->GetVulkanDevice(),
            &createInfo, nullptr, &m_VkPipelineCache));
    }

    void VulkanPipelineCache::Shutdown()
    {
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

        std::scoped_lock lock(m_Mutex);
        for (auto& [hash, entry] : m_Pipelines)
            vkDestroyPipeline(device, entry.Pipeline->GetVulkanPipeline(), nullptr);

        m_Pipelines.clear();

        if (m_VkPipelineCache)
        {
            vkDestroyPipelineCache(device, m_VkPipelineCache, nullptr);
            m_VkPipelineCache = VK_NULL_HANDLE;
        }
    }

    WeakRef<VulkanPipeline> VulkanPipelineCache::Get(const VulkanPipelineSpecification& spec)
    {
        uint64_t hash = Utils::CalculatePipelineSpecificationHash(spec);

        std::scoped_lock lock(m_Mutex);
        auto it = m_Pipelines.find(hash);
        if (it != m_Pipelines.end())
            return it->second.Pipeline;

        Ref<VulkanPipeline> pipeline = Create(spec);
        m_Pipelines.emplace(hash, Entry{ pipeline, spec.Shader.Raw() });
        return m_Pipelines[hash].Pipeline;
    }

    Ref<VulkanPipeline> VulkanPipelineCache::Create(const VulkanPipelineSpecification& spec)
    {
        return Ref<VulkanPipeline>::Create(spec, m_VkPipelineCache);
    }

    void VulkanPipelineCache::Erase(VulkanShader* shader)
    {
        std::scoped_lock lock(m_Mutex);
        for (auto it = m_Pipelines.begin(); it != m_Pipelines.end();)
        {
            if (it->second.Shader == shader)
                it = m_Pipelines.erase(it);
            else
                ++it;
        }
    }
}
