#include "prpch.h"
#include "VulkanShader.h"
#include "VulkanContext.h"
// #include "VulkanPipeline.h"

#include "Prism/Renderer/Renderer.h"

#include <map>

namespace Prism
{
    VulkanShader::VulkanShader(const std::vector<uint32_t>& spirvVertex, const std::vector<uint32_t>& spirvFragment,
        const PrismShaderCompiler::PassReflection& reflection)
        : m_Reflection(reflection)
    {
        CreateShaderStage(VK_SHADER_STAGE_VERTEX_BIT, spirvVertex);
        CreateShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, spirvFragment);
        CreateDescriptorSetLayouts();
        CreatePipelineLayout();
    }

    VulkanShader::~VulkanShader()
    {
        // 先入队销毁引用本 shader 的 pipeline，同队列 FIFO 保证先于 layout/module 执行
        // TODO: Pipeline还没有
        // VulkanPipelineCache::Erase(this);

        auto modules = m_ShaderModules;
        auto layouts = m_DescriptorSetLayouts;
        auto pipelineLayout = m_PipelineLayout;
        Renderer::SubmitResourceFree([modules, layouts, pipelineLayout]() {
            auto device = VulkanContext::GetCurrentDevice();
            PR_CORE_ASSERT(device, "VulkanShader::~VulkanShader: VulkanContext::GetCurrentDevice() returned nullptr");
            for (VkShaderModule module : modules)
                vkDestroyShaderModule(device->GetVulkanDevice(), module, nullptr);
            for (VkDescriptorSetLayout layout : layouts)
                vkDestroyDescriptorSetLayout(device->GetVulkanDevice(), layout, nullptr);
            vkDestroyPipelineLayout(device->GetVulkanDevice(), pipelineLayout, nullptr);
        });
    }

    void VulkanShader::CreateShaderStage(VkShaderStageFlagBits stage, const std::vector<uint32_t>& code)
    {
        PR_CORE_ASSERT(!code.empty(), "VulkanShader::CreateShaderStage: SPIR-V 为空!");

        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
        VkShaderModuleCreateInfo moduleInfo{};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = code.size() * sizeof(uint32_t);
        moduleInfo.pCode = code.data();
        VkShaderModule module = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleInfo, nullptr, &module));
        m_ShaderModules.push_back(module);

        VkPipelineShaderStageCreateInfo& stageInfo = m_ShaderStages.emplace_back();
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = stage;
        stageInfo.module = module;
        stageInfo.pName = "main";
    }

    void VulkanShader::CreateDescriptorSetLayouts()
    {
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

        // 反射按 set 分组、binding 升序，与 VulkanDescriptorSet::Bake 的 layout 定义保持一致
        std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> setBindings;
        for (const auto& desc : m_Reflection.Descriptors)
        {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = desc.Binding;
            switch (desc.Kind)
            {
            case PrismShaderCompiler::DescriptorKind::UniformBuffer:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case PrismShaderCompiler::DescriptorKind::StorageBuffer:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            case PrismShaderCompiler::DescriptorKind::Sampler:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            case PrismShaderCompiler::DescriptorKind::StorageImage:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            default:
                PR_CORE_ASSERT(false, "VulkanShader::CreateDescriptorSetLayouts: Unsupported descriptor kind!");
                continue;
            }
            // 布局兼容性含 stageFlags，反射的 StageFlags 是实际使用 stage（各 shader 不同），
            // 统一写死宽值与 VulkanDescriptorSet::Bake 对齐
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
            setBindings[desc.Set][desc.Binding] = layoutBinding;
        }

        if (setBindings.empty())
            return;

        // pipeline layout 的 setLayouts 必须稠密（set 0..maxSet），未声明的 set 用空 layout 占位
        uint32_t maxSet = setBindings.rbegin()->first;
        m_DescriptorSetLayouts.assign(maxSet + 1, VK_NULL_HANDLE);
        for (auto& [set, bindings] : setBindings)
        {
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
            layoutBindings.reserve(bindings.size());
            for (auto& [binding, layoutBinding] : bindings)
                layoutBindings.push_back(layoutBinding);

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
            layoutInfo.pBindings = layoutBindings.data();
            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayouts[set]));
        }

        VkDescriptorSetLayoutCreateInfo emptyInfo{};
        emptyInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        for (auto& layout : m_DescriptorSetLayouts)
        {
            if (layout)
                continue;
            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &emptyInfo, nullptr, &layout));
        }
    }

    void VulkanShader::CreatePipelineLayout()
    {
        // PrismDrawIndexPC{int}：shader 不声明 push_constant 时多出的 range 无害，统一声明
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(int32_t);

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
        layoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstantRange;
        VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::GetCurrentDevice()->GetVulkanDevice(),
            &layoutInfo, nullptr, &m_PipelineLayout));
    }
}
