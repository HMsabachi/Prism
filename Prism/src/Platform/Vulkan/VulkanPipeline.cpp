#include "prpch.h"
#include "VulkanPipeline.h"

#include "VulkanContext.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderer.h"
#include "VulkanShader.h"

#include "Prism/Renderer/Buffer/VertexBuffer.h"
#include "Prism/Renderer/Renderer.h"

#include <map>
#include <mutex>
#include <unordered_map>

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
            // TODO: Pipeline Hash未完成
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

    bool VulkanPSOKey::operator==(const VulkanPSOKey& other) const
    {
        if (Shader != other.Shader
            || VertexLayoutHash != other.VertexLayoutHash
            || ColorAttachmentCount != other.ColorAttachmentCount
            || Samples != other.Samples
            || Topology != other.Topology
            || AttachmentFormats != other.AttachmentFormats)
            return false;

        const auto& s = State;
        const auto& o = other.State;
        return s.BlendEnabled == o.BlendEnabled
            && s.SrcFactor == o.SrcFactor
            && s.DstFactor == o.DstFactor
            && s.SrcAlpha == o.SrcAlpha
            && s.DstAlpha == o.DstAlpha
            && s.DepthTest == o.DepthTest
            && s.DepthWrite == o.DepthWrite
            && s.DepthCompare == o.DepthCompare
            && s.WriteMask == o.WriteMask
            && s.DepthBiasFactor == o.DepthBiasFactor
            && s.DepthBiasUnits == o.DepthBiasUnits
            && s.Cull == o.Cull
            && s.StencilTest == o.StencilTest
            && s.StencilCompare == o.StencilCompare
            && s.StencilRef == o.StencilRef
            && s.StencilReadMask == o.StencilReadMask
            && s.StencilWriteMask == o.StencilWriteMask
            && s.StencilFailOp == o.StencilFailOp
            && s.StencilDepthFailOp == o.StencilDepthFailOp
            && s.StencilPassOp == o.StencilPassOp
            && s.FillMode == o.FillMode
            && s.LineWidth == o.LineWidth;
    }


    VulkanPipeline::VulkanPipeline(const VulkanPSOKey& key, const Ref<VulkanShader>& shader,
        std::span<const VertexBufferLayout> vertexLayouts, const Ref<VulkanFramebuffer>& framebuffer)
    {
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
        const auto& state = key.State;

        // 顶点输入：每个 vertex buffer 一个 binding，location 与 PSL codegen 的 SemanticToLocation 对齐
        PR_CORE_ASSERT(!vertexLayouts.empty(), "VulkanPipeline: vertexLayouts must not be empty");
        StaticVector<VkVertexInputBindingDescription, 8> bindingDescriptions;
        StaticVector<VkVertexInputAttributeDescription, 15> attributeDescriptions;
        size_t attributeCount = 0;
        for (const auto& layout : vertexLayouts)
            attributeCount += layout.GetElements().size();
        // bindingDescriptions.reserve(vertexLayouts.size());
        // attributeDescriptions.reserve(attributeCount);
        for (size_t i = 0; i < vertexLayouts.size(); i++)
        {
            const VertexBufferLayout& layout = vertexLayouts[i];
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
        inputAssemblyState.topology = Utils::ToVulkan(key.Topology);

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
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates(key.ColorAttachmentCount);
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
        multisampleState.rasterizationSamples = (VkSampleCountFlagBits)key.Samples;
        multisampleState.pSampleMask = nullptr;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = shader->GetPipelineLayout();
        pipelineInfo.renderPass = framebuffer->GetRenderPass();
        pipelineInfo.stageCount = static_cast<uint32_t>(shader->GetPipelineShaderStageCreateInfos().size());
        pipelineInfo.pStages = shader->GetPipelineShaderStageCreateInfos().data();
        pipelineInfo.pVertexInputState = &vertexInputState;
        pipelineInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineInfo.pRasterizationState = &rasterizationState;
        pipelineInfo.pColorBlendState = &colorBlendState;
        pipelineInfo.pMultisampleState = &multisampleState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pDynamicState = &dynamicState;

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VulkanContext::Get()->GetPipelineCache(), 1,
            &pipelineInfo, nullptr, &m_Pipeline));
        m_PipelineLayout = shader->GetPipelineLayout();
    }

    VulkanPipeline::~VulkanPipeline()
    {
        VkPipeline pipeline = m_Pipeline;
        Renderer::SubmitResourceFree([pipeline]()
        {
            auto device = VulkanContext::GetCurrentDevice();
            PR_CORE_ASSERT(device, "VulkanPipeline::~VulkanPipeline: VulkanContext::GetCurrentDevice() returned nullptr");
            vkDestroyPipeline(device->GetVulkanDevice(), pipeline, nullptr);
        });
    }

    void VulkanPipeline::RT_Bind() const
    {
        vkCmdBindPipeline(VulkanRenderer::GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }

    static std::mutex s_PipelineCacheMutex;
    static std::unordered_map<VulkanPSOKey, Ref<VulkanPipeline>> s_PipelineCache;

    Ref<VulkanPipeline> VulkanPipelineCache::Get(const Ref<VulkanShader>& shader,
        std::span<const VertexBufferLayout> vertexLayouts, const PrismShaderCompiler::PipelineState& state,
        const Ref<VulkanFramebuffer>& framebuffer, PrimitiveType topology)
    {
        VulkanPSOKey key;
        key.Shader = const_cast<VulkanShader*>(shader.Raw());
        key.VertexLayoutHash = Utils::CombineLayoutHash(vertexLayouts);
        key.State = state;
        key.ColorAttachmentCount = static_cast<uint32_t>(framebuffer->GetColorAttachmentCount());
        key.Samples = framebuffer->GetSamples();
        key.AttachmentFormats = framebuffer->GetAttachmentFormats();
        key.Topology = topology;

        std::scoped_lock lock(s_PipelineCacheMutex);
        auto it = s_PipelineCache.find(key);
        if (it != s_PipelineCache.end())
            return it->second;

        Ref<VulkanPipeline> pipeline = Create(key, shader, vertexLayouts, framebuffer);
        s_PipelineCache.emplace(key, pipeline);
        return pipeline;
    }

    Ref<VulkanPipeline> VulkanPipelineCache::Create(const VulkanPSOKey& key, const Ref<VulkanShader>& shader,
        std::span<const VertexBufferLayout> vertexLayouts, const Ref<VulkanFramebuffer>& framebuffer)
    {
        return Ref<VulkanPipeline>::Create(key, shader, vertexLayouts, framebuffer);
    }

    void VulkanPipelineCache::Erase(VulkanShader* shader)
    {
        std::scoped_lock lock(s_PipelineCacheMutex);
        for (auto it = s_PipelineCache.begin(); it != s_PipelineCache.end();)
        {
            if (it->first.Shader == shader)
                it = s_PipelineCache.erase(it);
            else
                ++it;
        }
    }

    void VulkanPipelineCache::Clear()
    {
        std::scoped_lock lock(s_PipelineCacheMutex);
        for (auto& [key, pipeline] : s_PipelineCache)
            vkDestroyPipeline(VulkanContext::GetCurrentDevice()->GetVulkanDevice(),
                pipeline->m_Pipeline, nullptr);
        s_PipelineCache.clear();
    }
}

namespace std
{
    size_t hash<Prism::VulkanPSOKey>::operator()(const Prism::VulkanPSOKey& key) const noexcept
    {
        const auto& s = key.State;
        size_t seed = hash<Prism::VulkanShader*>{}(key.Shader);
        auto combine = [&seed](auto v)
        {
            seed ^= hash<decltype(v)>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        combine(key.VertexLayoutHash);
        combine(key.ColorAttachmentCount);
        combine(key.Samples);
        combine((uint32_t)key.Topology);
        for (VkFormat format : key.AttachmentFormats)
            combine((uint32_t)format);
        combine(s.BlendEnabled);
        combine(s.SrcFactor);
        combine(s.DstFactor);
        combine(s.SrcAlpha);
        combine(s.DstAlpha);
        combine(s.DepthTest);
        combine(s.DepthWrite);
        combine(s.DepthCompare);
        combine(s.WriteMask);
        combine(s.DepthBiasFactor);
        combine(s.DepthBiasUnits);
        combine(s.Cull);
        combine(s.StencilTest);
        combine(s.StencilCompare);
        combine(s.StencilRef);
        combine(s.StencilReadMask);
        combine(s.StencilWriteMask);
        combine(s.StencilFailOp);
        combine(s.StencilDepthFailOp);
        combine(s.StencilPassOp);
        combine(s.FillMode);
        combine(s.LineWidth);
        return seed;
    }
}
