#include "prpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanContext.h"

#include "VulkanUniformBuffer.h"
#include "VulkanImage.h"
#include "VulkanShaderStorageBuffer.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Utilities/StaticVector.h"

namespace Prism
{


    VulkanDescriptorSet::~VulkanDescriptorSet()
    {
        auto device = VulkanContext::GetCurrentDevice();
        if (!device)
            return;

        if (m_DescriptorPool)
            vkDestroyDescriptorPool(device->GetVulkanDevice(), m_DescriptorPool, nullptr);
        if (m_DescriptorSetLayout)
            vkDestroyDescriptorSetLayout(device->GetVulkanDevice(), m_DescriptorSetLayout, nullptr);
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanUniformBuffer> buffer)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::UniformBuffer;
        bd.Resource = buffer;
        m_Dirty = true;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanShaderStorageBuffer> buffer)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::StorageBuffer;
        bd.Resource = buffer;
        m_Dirty = true;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanImage2D> image)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::Image2D;
        bd.Resource = image;
        m_Dirty = true;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanImageCube> image)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::ImageCube;
        bd.Resource = image;
        m_Dirty = true;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::Bake()
    {
        // Create Descriptor Pool
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
        if (m_DescriptorPool)
        {
            vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
            m_DescriptorPool = VK_NULL_HANDLE;
        }
        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
        };
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = VulkanFramesInFlight;
        poolInfo.poolSizeCount = 10;
        poolInfo.pPoolSizes = poolSizes;
        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool));
        StaticVector<VkDescriptorSetLayoutBinding, 16> bindings;
        for (auto& [binding, bd] : m_Bindings)
        {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding;
            switch (bd.Type)
            {
            case RenderResourceType::UniformBuffer:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case RenderResourceType::StorageBuffer:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            case RenderResourceType::Image2D:
            case RenderResourceType::ImageCube:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            default:
                PR_CORE_ERROR("Unsupported resource type in VulkanDescriptorSet::Bake");
                continue;
            }
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
            bindings.emplace_back(layoutBinding);
        }
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayout));
        StaticVector<VkDescriptorSetLayout, VulkanFramesInFlight> layouts;
        for (uint32_t i = 0; i < VulkanFramesInFlight; i++) layouts.emplace_back(m_DescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = VulkanFramesInFlight;
        allocInfo.pSetLayouts = layouts.data();
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, m_DescriptorSets.data()));
    }

    void VulkanDescriptorSet::RT_Prepare()
    {

    }

    VkDescriptorSet VulkanDescriptorSet::GetDescriptorSet() const
    {
        return m_DescriptorSets.empty() ? VK_NULL_HANDLE : m_DescriptorSets[0];
    }

    uint32_t VulkanDescriptorSet::CurrentSlotIndex() const
    {
        return Renderer::RT_GetCurrentFrameIndex() % VulkanFramesInFlight;
    }

}
