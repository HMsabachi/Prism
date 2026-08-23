#include "prpch.h"
#include "VulkanDescriptorSetManager.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanImage.h"

namespace Prism
{
    VulkanDescriptorSetManager::VulkanDescriptorSetManager(uint32_t setCount, uint32_t framesInFlight)
    {
        Init(setCount, framesInFlight);
    }

    VulkanDescriptorSetManager::~VulkanDescriptorSetManager()
    {
        Release();
    }

    void VulkanDescriptorSetManager::Init(uint32_t setCount, uint32_t framesInFlight)
    {
        Release();
        m_SetCount = setCount;
        m_FramesInFlight = framesInFlight;
        m_Sets.resize(setCount);
    }

    void VulkanDescriptorSetManager::Release()
    {
        auto device = VulkanContext::GetCurrentDevice();
        if (device && m_DescriptorPool)
            vkDestroyDescriptorPool(device->GetVulkanDevice(), m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
        m_Sets.clear();
        m_SetCount = 0;
        m_FramesInFlight = 0;
    }

    void VulkanDescriptorSetManager::SetLayout(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    {
        PR_CORE_ASSERT(set < m_Sets.size());
        auto& setState = m_Sets[set];

        auto device = VulkanContext::GetCurrentDevice();
        VkDevice vkDevice = device->GetVulkanDevice();

        if (setState.Layout)
        {
            vkDestroyDescriptorSetLayout(vkDevice, setState.Layout, nullptr);
            setState.Layout = VK_NULL_HANDLE;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = (uint32_t)bindings.size();
        layoutInfo.pBindings = bindings.data();
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &setState.Layout));

        setState.PoolSizes.clear();
        for (const auto& binding : bindings)
        {
            auto it = std::find_if(setState.PoolSizes.begin(), setState.PoolSizes.end(), [&](const VkDescriptorPoolSize& size)
            {
                return size.type == binding.descriptorType;
            });
            if (it == setState.PoolSizes.end())
                setState.PoolSizes.push_back({ binding.descriptorType, binding.descriptorCount * m_FramesInFlight });
            else
                it->descriptorCount += binding.descriptorCount * m_FramesInFlight;
        }
    }

    void VulkanDescriptorSetManager::SetInput(uint32_t set, uint32_t binding, const Ref<UniformBuffer>& uniformBuffer)
    {
        auto& bindingState = m_Sets.at(set).Bindings[binding];
        bindingState.Input.Type = VulkanDescriptorResourceType::UniformBuffer;
        bindingState.Input.Resources = { uniformBuffer };
        bindingState.Dirty = true;
    }

    void VulkanDescriptorSetManager::SetInput(uint32_t set, uint32_t binding, const Ref<ShaderStorageBuffer>& storageBuffer)
    {
        auto& bindingState = m_Sets.at(set).Bindings[binding];
        bindingState.Input.Type = VulkanDescriptorResourceType::StorageBuffer;
        bindingState.Input.Resources = { storageBuffer };
        bindingState.Dirty = true;
    }

    void VulkanDescriptorSetManager::SetInput(uint32_t set, uint32_t binding, const Ref<Texture2D>& texture)
    {
        auto& bindingState = m_Sets.at(set).Bindings[binding];
        bindingState.Input.Type = VulkanDescriptorResourceType::Texture2D;
        bindingState.Input.Resources = { texture };
        bindingState.Dirty = true;
    }

    void VulkanDescriptorSetManager::SetInput(uint32_t set, uint32_t binding, const Ref<TextureCube>& texture)
    {
        auto& bindingState = m_Sets.at(set).Bindings[binding];
        bindingState.Input.Type = VulkanDescriptorResourceType::TextureCube;
        bindingState.Input.Resources = { texture };
        bindingState.Dirty = true;
    }

    void VulkanDescriptorSetManager::SetInput(uint32_t set, uint32_t binding, const Ref<Image2D>& image)
    {
        auto& bindingState = m_Sets.at(set).Bindings[binding];
        bindingState.Input.Type = VulkanDescriptorResourceType::Image2D;
        bindingState.Input.Resources = { image };
        bindingState.Dirty = true;
    }

    void VulkanDescriptorSetManager::Bake()
    {
        auto device = VulkanContext::GetCurrentDevice();
        VkDevice vkDevice = device->GetVulkanDevice();

        if (m_DescriptorPool)
            vkDestroyDescriptorPool(vkDevice, m_DescriptorPool, nullptr);

        std::vector<VkDescriptorPoolSize> poolSizes;
        std::vector<VkDescriptorSetLayout> layouts;
        layouts.reserve(m_Sets.size());
        for (auto& setState : m_Sets)
        {
            layouts.push_back(setState.Layout);
            for (const auto& poolSize : setState.PoolSizes)
            {
                auto it = std::find_if(poolSizes.begin(), poolSizes.end(), [&](const VkDescriptorPoolSize& size)
                {
                    return size.type == poolSize.type;
                });
                if (it == poolSizes.end())
                    poolSizes.push_back(poolSize);
                else
                    it->descriptorCount += poolSize.descriptorCount;
            }
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = m_SetCount * m_FramesInFlight;
        poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        VK_CHECK_RESULT(vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &m_DescriptorPool));

        for (auto& setState : m_Sets)
        {
            setState.DescriptorSets.resize(m_FramesInFlight);
            std::vector<VkDescriptorSetLayout> frameLayouts(m_FramesInFlight, setState.Layout);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_DescriptorPool;
            allocInfo.descriptorSetCount = m_FramesInFlight;
            allocInfo.pSetLayouts = frameLayouts.data();
            VK_CHECK_RESULT(vkAllocateDescriptorSets(vkDevice, &allocInfo, setState.DescriptorSets.data()));
            for (auto& [binding, bindingState] : setState.Bindings)
                bindingState.Dirty = true;
        }
    }

    void VulkanDescriptorSetManager::MarkDirty(uint32_t set, uint32_t binding)
    {
        m_Sets.at(set).Bindings.at(binding).Dirty = true;
    }

    void VulkanDescriptorSetManager::Prepare()
    {
        auto materialDevice = VulkanContext::GetCurrentDevice();
        VkDevice vkDevice = materialDevice->GetVulkanDevice();

        for (uint32_t set = 0; set < m_Sets.size(); set++)
        {
            auto& setState = m_Sets[set];
            for (auto& [binding, bindingState] : setState.Bindings)
            {
                RefreshDirtyState(bindingState);
                if (!bindingState.Dirty)
                    continue;

                RefreshBindingCache(bindingState);
                for (uint32_t frame = 0; frame < m_FramesInFlight; frame++)
                    WriteBinding(setState, bindingState, setState.DescriptorSets[frame], binding);
                bindingState.Dirty = false;
            }
        }
    }

    VkDescriptorSet VulkanDescriptorSetManager::GetDescriptorSet(uint32_t set, uint32_t frameIndex) const
    {
        PR_CORE_ASSERT(set < m_Sets.size());
        return m_Sets[set].DescriptorSets.at(frameIndex);
    }

    const std::vector<VkDescriptorSet>& VulkanDescriptorSetManager::GetDescriptorSets(uint32_t set) const
    {
        return m_Sets.at(set).DescriptorSets;
    }

    bool VulkanDescriptorSetManager::HasDescriptorSets() const
    {
        if (m_Sets.empty())
            return false;
        return !m_Sets[0].DescriptorSets.empty();
    }

    bool VulkanDescriptorSetManager::IsDirty(uint32_t set, uint32_t binding) const
    {
        return m_Sets.at(set).Bindings.at(binding).Dirty;
    }

    void VulkanDescriptorSetManager::WriteBinding(SetState& setState, BindingState& bindingState, VkDescriptorSet descriptorSet, uint32_t binding) const
    {
        VkWriteDescriptorSet write = bindingState.Write;
        write.dstSet = descriptorSet;
        write.dstBinding = binding;
        vkUpdateDescriptorSets(VulkanContext::GetCurrentDevice()->GetVulkanDevice(), 1, &write, 0, nullptr);
    }

    void VulkanDescriptorSetManager::RefreshDirtyState(BindingState& bindingState)
    {
        if (bindingState.CachedHandles.empty())
        {
            bindingState.Dirty = true;
            return;
        }

        switch (bindingState.Input.Type)
        {
        case VulkanDescriptorResourceType::UniformBuffer:
        {
            auto ubo = bindingState.Input.Resources[0].As<UniformBuffer>();
            bindingState.Dirty = GetHandle(ubo) != bindingState.CachedHandles[0];
            break;
        }
        case VulkanDescriptorResourceType::StorageBuffer:
        {
            auto ssbo = bindingState.Input.Resources[0].As<ShaderStorageBuffer>();
            bindingState.Dirty = GetHandle(ssbo) != bindingState.CachedHandles[0];
            break;
        }
        case VulkanDescriptorResourceType::Texture2D:
        {
            auto tex = bindingState.Input.Resources[0].As<Texture2D>();
            bindingState.Dirty = GetHandle(tex) != bindingState.CachedHandles[0];
            break;
        }
        case VulkanDescriptorResourceType::TextureCube:
        {
            auto tex = bindingState.Input.Resources[0].As<TextureCube>();
            bindingState.Dirty = GetHandle(tex) != bindingState.CachedHandles[0];
            break;
        }
        case VulkanDescriptorResourceType::Image2D:
        {
            auto img = bindingState.Input.Resources[0].As<Image2D>();
            bindingState.Dirty = GetHandle(img) != bindingState.CachedHandles[0];
            break;
        }
        default:
            bindingState.Dirty = true;
            break;
        }
    }

    void VulkanDescriptorSetManager::RefreshBindingCache(BindingState& bindingState)
    {
        auto& input = bindingState.Input;
        switch (input.Type)
        {
        case VulkanDescriptorResourceType::UniformBuffer:
        {
            auto ubo = input.Resources[0].As<UniformBuffer>();
            auto vkUbo = ubo.As<VulkanUniformBuffer>();
            bindingState.Write = {};
            bindingState.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            bindingState.Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindingState.Write.descriptorCount = 1;
            bindingState.BufferInfo = vkUbo->GetDescriptorBufferInfo();
            bindingState.Write.pBufferInfo = &bindingState.BufferInfo;
            bindingState.CachedHandles = { GetHandle(ubo) };
            break;
        }
        case VulkanDescriptorResourceType::StorageBuffer:
        {
            auto ssbo = input.Resources[0].As<ShaderStorageBuffer>();
            bindingState.Write = {};
            bindingState.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            bindingState.Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            bindingState.Write.descriptorCount = 1;
            bindingState.CachedHandles = { GetHandle(ssbo) };
            break;
        }
        case VulkanDescriptorResourceType::Texture2D:
        {
            auto tex = input.Resources[0].As<Texture2D>();
            bindingState.Write = {};
            bindingState.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            bindingState.Write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindingState.Write.descriptorCount = 1;
            bindingState.ImageInfo = GetImageInfo(tex);
            bindingState.Write.pImageInfo = &bindingState.ImageInfo;
            bindingState.CachedHandles = { GetHandle(tex) };
            break;
        }
        case VulkanDescriptorResourceType::TextureCube:
        {
            auto tex = input.Resources[0].As<TextureCube>();
            bindingState.Write = {};
            bindingState.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            bindingState.Write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindingState.Write.descriptorCount = 1;
            bindingState.ImageInfo = GetImageInfo(tex);
            bindingState.Write.pImageInfo = &bindingState.ImageInfo;
            bindingState.CachedHandles = { GetHandle(tex) };
            break;
        }
        case VulkanDescriptorResourceType::Image2D:
        {
            auto img = input.Resources[0].As<Image2D>();
            bindingState.Write = {};
            bindingState.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            bindingState.Write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            bindingState.Write.descriptorCount = 1;
            bindingState.ImageInfo = GetImageInfo(img);
            bindingState.Write.pImageInfo = &bindingState.ImageInfo;
            bindingState.CachedHandles = { GetHandle(img) };
            break;
        }
        default:
            break;
        }
    }

    uint64_t VulkanDescriptorSetManager::GetHandle(const Ref<UniformBuffer>& uniformBuffer) const
    {
        return uniformBuffer ? (uint64_t)uniformBuffer->GetSize() : 0;
    }

    uint64_t VulkanDescriptorSetManager::GetHandle(const Ref<ShaderStorageBuffer>& storageBuffer) const
    {
        return storageBuffer ? (uint64_t)storageBuffer->GetSize() : 0;
    }

    uint64_t VulkanDescriptorSetManager::GetHandle(const Ref<Texture2D>& texture) const
    {
        return texture ? texture->GetHash() : 0;
    }

    uint64_t VulkanDescriptorSetManager::GetHandle(const Ref<TextureCube>& texture) const
    {
        return texture ? texture->GetHash() : 0;
    }

    uint64_t VulkanDescriptorSetManager::GetHandle(const Ref<Image2D>& image) const
    {
        return image ? image->GetHash() : 0;
    }

    VkDescriptorImageInfo VulkanDescriptorSetManager::GetImageInfo(const Ref<Texture2D>& texture) const
    {
        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        if (texture && texture->GetType() == TextureType::Texture2D)
            info = texture.As<VulkanTexture2D>()->GetVulkanDescriptorInfo();
        return info;
    }

    VkDescriptorImageInfo VulkanDescriptorSetManager::GetImageInfo(const Ref<TextureCube>& texture) const
    {
        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        if (texture && texture->GetType() == TextureType::TextureCube)
            info = texture.As<VulkanTextureCube>()->GetVulkanDescriptorInfo();
        return info;
    }

    VkDescriptorImageInfo VulkanDescriptorSetManager::GetImageInfo(const Ref<Image2D>& image) const
    {
        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        if (image)
            info = image.As<VulkanImage2D>()->GetDescriptor();
        return info;
    }
}
