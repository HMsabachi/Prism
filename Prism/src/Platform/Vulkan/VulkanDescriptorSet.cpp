#include "prpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanContext.h"

#include <mutex>

#include "VulkanUniformBuffer.h"
#include "VulkanImage.h"
#include "VulkanTexture.h"
#include "VulkanShaderStorageBuffer.h"
#include "VulkanRenderer.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Utilities/StaticVector.h"

namespace Prism
{
    namespace
    {
        struct DescriptorPoolBucket
        {
            VkDescriptorPool Pool = VK_NULL_HANDLE;
            uint32_t UsedSets = 0;
        };
        constexpr uint32_t s_DescriptorPoolSetCapacity = 256;
        constexpr uint32_t s_MaxBindingsPerSet = 16;
        std::mutex s_DescriptorPoolMutex;
        StaticVector<DescriptorPoolBucket, 10> s_DescriptorPools;
    }

    VkDescriptorPool VulkanGlobalDescriptorPool::Allocate(VkDescriptorSetLayout layout, uint32_t setCount, VkDescriptorSet* outSets)
    {
        std::scoped_lock lock(s_DescriptorPoolMutex);
        DescriptorPoolBucket* bucket = nullptr;
        for (auto it = s_DescriptorPools.rbegin(); it != s_DescriptorPools.rend(); ++it)
        {
            if (it->UsedSets + setCount <= s_DescriptorPoolSetCapacity)
            {
                bucket = &(*it);
                break;
            }
        }
        if (!bucket)
        {
            VkDescriptorPoolSize poolSizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, s_DescriptorPoolSetCapacity * s_MaxBindingsPerSet }
            };
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolInfo.maxSets = s_DescriptorPoolSetCapacity;
            poolInfo.poolSizeCount = 11;
            poolInfo.pPoolSizes = poolSizes;
            VkDescriptorPool pool = VK_NULL_HANDLE;
            VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::GetCurrentDevice()->GetVulkanDevice(), &poolInfo, nullptr, &pool));
            s_DescriptorPools.push_back({ pool, 0 });
            bucket = &s_DescriptorPools.back();
        }
        std::vector<VkDescriptorSetLayout> layouts(setCount, layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = bucket->Pool;
        allocInfo.descriptorSetCount = setCount;
        allocInfo.pSetLayouts = layouts.data();
        VK_CHECK_RESULT(vkAllocateDescriptorSets(VulkanContext::GetCurrentDevice()->GetVulkanDevice(), &allocInfo, outSets));
        bucket->UsedSets += setCount;
        return bucket->Pool;
    }

    void VulkanGlobalDescriptorPool::Free(VkDescriptorPool pool, uint32_t setCount, const VkDescriptorSet* sets)
    {
        std::scoped_lock lock(s_DescriptorPoolMutex);

        vkFreeDescriptorSets(VulkanContext::GetCurrentDevice()->GetVulkanDevice(), pool, setCount, sets);
        for (auto& bucket : s_DescriptorPools)
        {
            if (bucket.Pool == pool)
            {
                PR_CORE_ASSERT(bucket.UsedSets >= setCount, "VulkanGlobalDescriptorPool::Free: set count underflow");
                bucket.UsedSets -= setCount;
                break;
            }
        }
    }

    void VulkanGlobalDescriptorPool::Shutdown()
    {
        std::scoped_lock lock(s_DescriptorPoolMutex);

        if (!VulkanContext::GetCurrentDevice())
            return;
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
        for (auto& bucket : s_DescriptorPools)
        {
            if (bucket.Pool)
                vkDestroyDescriptorPool(device, bucket.Pool, nullptr);
        }
        s_DescriptorPools.clear();
    }

    VulkanDescriptorSet::~VulkanDescriptorSet()
    {
        Reset();
    }

    void VulkanDescriptorSet::Reset()
    {
        if (m_SourcePool)
        {
            auto sourcePool = m_SourcePool;
            auto descriptorSetLayout = m_DescriptorSetLayout;
            auto descriptorSets = m_DescriptorSets;
            Renderer::SubmitResourceFree([sourcePool, descriptorSetLayout, descriptorSets] {
                auto device = VulkanContext::GetCurrentDevice();
                PR_CORE_ASSERT(device, "VulkanDescriptorSet::Reset: VulkanContext::GetCurrentDevice() returned nullptr");
                VulkanGlobalDescriptorPool::Free(sourcePool, VulkanFramesInFlight, descriptorSets.data());
                vkDestroyDescriptorSetLayout(device->GetVulkanDevice(), descriptorSetLayout, nullptr);
            });
        }
        m_SourcePool = VK_NULL_HANDLE;
        m_DescriptorSetLayout = VK_NULL_HANDLE;
        m_DescriptorSets.fill(VK_NULL_HANDLE);
        m_IsBaked = false;
        m_Bindings.clear();
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

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanImage2D> image, uint32_t level)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::StorageImage2D;
        bd.Resource = image;
        bd.Level = level;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::SetInput(uint32_t binding, Ref<VulkanImageCube> image, uint32_t level)
    {
        Binding bd = {};
        bd.Type = RenderResourceType::StorageImageCube;
        bd.Resource = image;
        bd.Level = level;
        m_Bindings[binding] = bd;
    }

    void VulkanDescriptorSet::Bake()
    {
        PR_CORE_ASSERT(!m_IsBaked, "VulkanDescriptorSet::Bake: Descriptor set is already baked!");
        m_IsBaked = true;
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
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
            case RenderResourceType::StorageImage2D:
            case RenderResourceType::StorageImageCube:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
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
        m_SourcePool = VulkanGlobalDescriptorPool::Allocate(m_DescriptorSetLayout, VulkanFramesInFlight, m_DescriptorSets.data());
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
        PR_CORE_ASSERT(m_IsBaked);
        StaticVector<VkWriteDescriptorSet, 16> writes;
        StaticVector<ShouldWriteInfo, 16> shouldWrites;
        for (auto& [binding, bd] : m_Bindings)
        {
            switch (bd.Type)
            {
            case RenderResourceType::UniformBuffer:
            {
                WeakRef<VulkanUniformBuffer> ubo = bd.Resource.As<VulkanUniformBuffer>();
                if (!ubo) ubo = VulkanRenderer::RT_GetEmptyUniformBuffer();
                VkDescriptorBufferInfo bufferInfo = ubo->GetDescriptor();
                if (bufferInfo.buffer == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = bufferInfo.buffer;
                shouldWrites.emplace_back(binding, bufferInfo, VkDescriptorImageInfo{}, bd.Type);
                break;
            }
            case RenderResourceType::StorageBuffer:
            {
                WeakRef<VulkanShaderStorageBuffer> ssbo = bd.Resource.As<VulkanShaderStorageBuffer>();
                if (!ssbo) ssbo = VulkanRenderer::RT_GetEmptyShaderStorageBuffer();
                VkDescriptorBufferInfo bufferInfo = ssbo->GetDescriptor();
                if (bufferInfo.buffer == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = bufferInfo.buffer;
                shouldWrites.emplace_back(binding, bufferInfo, VkDescriptorImageInfo{}, bd.Type);
                break;
            }
            case RenderResourceType::Image2D:
            {
                WeakRef<VulkanImage2D> image = bd.Resource.As<VulkanImage2D>();
                if (!image) image = VulkanRenderer::RT_GetBlackImage2D();
                VkDescriptorImageInfo imageInfo = image->GetDescriptor();
                if (imageInfo.imageView == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = imageInfo.imageView;
                shouldWrites.emplace_back(binding, VkDescriptorBufferInfo{}, imageInfo, bd.Type);
                break;
            }
            case RenderResourceType::ImageCube:
            {
                WeakRef<VulkanImageCube> image = bd.Resource.As<VulkanImageCube>();
                if (!image) image = VulkanRenderer::RT_GetBlackImageCube();
                VkDescriptorImageInfo imageInfo = image->GetDescriptor();
                if (imageInfo.imageView == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = imageInfo.imageView;
                shouldWrites.emplace_back(binding, VkDescriptorBufferInfo{}, imageInfo, bd.Type);
                break;
            }
            case RenderResourceType::StorageImage2D:
            {
                WeakRef<VulkanImage2D> image = bd.Resource.As<VulkanImage2D>();
                if (!image) image = VulkanRenderer::RT_GetBlackImage2D();
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageView = image->GetOrCreateStorageImageView(image ? bd.Level : 0);
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                if (imageInfo.imageView == bd.NativeHandle[CurrentSlotIndex()]) continue;
                bd.NativeHandle[CurrentSlotIndex()] = imageInfo.imageView;
                shouldWrites.emplace_back(binding, VkDescriptorBufferInfo{}, imageInfo, bd.Type);
                break;
            }
            case RenderResourceType::StorageImageCube:
            {
                WeakRef<VulkanImageCube> image = bd.Resource.As<VulkanImageCube>();
                if (!image) image = VulkanRenderer::RT_GetBlackImageCube();
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageView = image->GetOrCreateStorageImageView(image ? bd.Level : 0);
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
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
            case RenderResourceType::StorageImage2D:
            case RenderResourceType::StorageImageCube:
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
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
