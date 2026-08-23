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
        auto descriptorPool = m_DescriptorPool;
        auto descriptorSetLayout = m_DescriptorSetLayout;
        Renderer::SubmitResourceFree([descriptorPool, descriptorSetLayout] {
            auto device = VulkanContext::GetCurrentDevice();
            PR_CORE_ASSERT(device, "VulkanDescriptorSet::~VulkanDescriptorSet: VulkanContext::GetCurrentDevice() returned nullptr");
            if (descriptorPool)
                vkDestroyDescriptorPool(device->GetVulkanDevice(), descriptorPool, nullptr);
            if (descriptorSetLayout)
                vkDestroyDescriptorSetLayout(device->GetVulkanDevice(), descriptorSetLayout, nullptr);
        });
        
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanUniformBuffer> buffer)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::UniformBuffer;
        bd.Resource = buffer;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanShaderStorageBuffer> buffer)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::StorageBuffer;
        bd.Resource = buffer;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanImage2D> image)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::Image2D;
        bd.Resource = image;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanImageCube> image)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::ImageCube;
        bd.Resource = image;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::Bake()
    {
        PR_CORE_ASSERT(!m_IsBaked, "VulkanDescriptorSet::Bake: Descriptor set is already baked!");
        m_IsBaked = true;
        // Create Descriptor Pool
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
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
        poolInfo.poolSizeCount = 11;
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
        struct ShouldWriteInfo
        {
            uint32_t binding;
            VkDescriptorBufferInfo bufferInfo;
            VkDescriptorImageInfo imageInfo;
            RenderResourceType type;
        };
        StaticVector<VkWriteDescriptorSet, 16> writes;
        StaticVector<ShouldWriteInfo, 16> shouldWrites;
        for (auto& [binding, bd] : m_Bindings)
        {
            if (!bd.Resource || bd.Type == RenderResourceType::None) continue;
            switch (bd.Type)
            {
            case RenderResourceType::UniformBuffer:
            {
                Ref<VulkanUniformBuffer> ubo = bd.Resource.As<VulkanUniformBuffer>();
                VkDescriptorBufferInfo bufferInfo = ubo->GetDescriptor(CurrentSlotIndex());
                if (bufferInfo.buffer == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = bufferInfo.buffer;
                shouldWrites.emplace_back(binding, bufferInfo, VkDescriptorImageInfo{}, bd.Type);
                break;
            }
            case RenderResourceType::StorageBuffer:
            {
                Ref<VulkanShaderStorageBuffer> ssbo = bd.Resource.As<VulkanShaderStorageBuffer>();
                VkDescriptorBufferInfo bufferInfo = ssbo->GetDescriptor(CurrentSlotIndex());
                if (bufferInfo.buffer == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = bufferInfo.buffer;
                shouldWrites.emplace_back(binding, bufferInfo, VkDescriptorImageInfo{}, bd.Type);
                break;
            }
            case RenderResourceType::Image2D:
            {
                Ref<VulkanImage2D> image = bd.Resource.As<VulkanImage2D>();
                VkDescriptorImageInfo imageInfo = image->GetDescriptor();
                if (imageInfo.imageView == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = imageInfo.imageView;
                shouldWrites.emplace_back(binding, VkDescriptorBufferInfo{}, imageInfo, bd.Type);
                break;
            }
            case RenderResourceType::ImageCube:
            {
                Ref<VulkanImageCube> image = bd.Resource.As<VulkanImageCube>();
                VkDescriptorImageInfo imageInfo = image->GetDescriptor();
                if (imageInfo.imageView == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = imageInfo.imageView;
                shouldWrites.emplace_back(binding, VkDescriptorBufferInfo{}, imageInfo, bd.Type);
                break;
            }
            default: PR_CORE_ASSERT(false, "Unsupported resource type in VulkanDescriptorSet::RT_Prepare"); break;
            }
        }
        if (shouldWrites.empty()) return;
        for (auto& info : shouldWrites)
        {
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = m_DescriptorSets[CurrentSlotIndex()];
            write.dstBinding = info.binding;
            write.descriptorCount = 1;
            switch (info.type)
            {
            case RenderResourceType::UniformBuffer:
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.pBufferInfo = &info.bufferInfo;
                break;
            case RenderResourceType::StorageBuffer:
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                write.pBufferInfo = &info.bufferInfo;
                break;
            case RenderResourceType::Image2D:
            case RenderResourceType::ImageCube:
                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.pImageInfo = &info.imageInfo;
                break;
            default: PR_CORE_ASSERT(false, "Unsupported resource type in VulkanDescriptorSet::RT_Prepare"); break;
            }
            writes.emplace_back(write);
        }
        vkUpdateDescriptorSets(VulkanContext::GetCurrentDevice()->GetVulkanDevice(), (uint32_t)writes.size(), writes.data(), 0, nullptr);

    }


    uint32_t VulkanDescriptorSet::CurrentSlotIndex() const
    {
        return Renderer::RT_GetCurrentFrameIndex() % VulkanFramesInFlight;
    }

}
