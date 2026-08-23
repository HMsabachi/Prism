#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"
#include "Platform/Vulkan/Vulkan.h"

#include <map>
#include <vector>

namespace Prism
{
    enum class VulkanDescriptorResourceType : uint16_t
    {
        None = 0,
        UniformBuffer,
        StorageBuffer,
        Texture2D,
        TextureCube,
        Image2D
    };

    struct VulkanDescriptorInput
    {
        VulkanDescriptorResourceType Type = VulkanDescriptorResourceType::None;
        std::vector<Ref<RefCounted>> Resources;
    };

    struct VulkanDescriptorBindingDeclaration
    {
        VulkanDescriptorResourceType Type = VulkanDescriptorResourceType::None;
        uint32_t Binding = 0;
        uint32_t Count = 0;
    };

    class VulkanDescriptorSetManager
    {
    public:
        VulkanDescriptorSetManager() = default;
        VulkanDescriptorSetManager(uint32_t setCount, uint32_t framesInFlight);
        ~VulkanDescriptorSetManager();

        void Init(uint32_t setCount, uint32_t framesInFlight);
        void Release();

        void SetLayout(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        void SetInput(uint32_t set, uint32_t binding, const Ref<UniformBuffer>& uniformBuffer);
        void SetInput(uint32_t set, uint32_t binding, const Ref<ShaderStorageBuffer>& storageBuffer);
        void SetInput(uint32_t set, uint32_t binding, const Ref<Texture2D>& texture);
        void SetInput(uint32_t set, uint32_t binding, const Ref<TextureCube>& texture);
        void SetInput(uint32_t set, uint32_t binding, const Ref<Image2D>& image);

        void Bake();
        void MarkDirty(uint32_t set, uint32_t binding);
        void Prepare();

        VkDescriptorSet GetDescriptorSet(uint32_t set, uint32_t frameIndex = 0) const;
        const std::vector<VkDescriptorSet>& GetDescriptorSets(uint32_t set) const;

        bool HasDescriptorSets() const;
        bool IsDirty(uint32_t set, uint32_t binding) const;

    private:
        struct BindingState
        {
            VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
            VulkanDescriptorInput Input;
            bool Dirty = true;
            std::vector<uint64_t> CachedHandles;
            VkWriteDescriptorSet Write{};
            VkDescriptorBufferInfo BufferInfo{};
            VkDescriptorImageInfo ImageInfo{};
        };

        struct SetState
        {
            VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
            std::vector<VkDescriptorPoolSize> PoolSizes;
            std::map<uint32_t, BindingState> Bindings;
            std::vector<VkDescriptorSet> DescriptorSets;
        };

        void WriteBinding(SetState& setState, BindingState& bindingState, VkDescriptorSet descriptorSet, uint32_t binding) const;
        void RefreshBindingCache(BindingState& bindingState);
        void RefreshDirtyState(BindingState& bindingState);
        uint64_t GetHandle(const Ref<UniformBuffer>& uniformBuffer) const;
        uint64_t GetHandle(const Ref<ShaderStorageBuffer>& storageBuffer) const;
        uint64_t GetHandle(const Ref<Texture2D>& texture) const;
        uint64_t GetHandle(const Ref<TextureCube>& texture) const;
        uint64_t GetHandle(const Ref<Image2D>& image) const;
        VkDescriptorImageInfo GetImageInfo(const Ref<Texture2D>& texture) const;
        VkDescriptorImageInfo GetImageInfo(const Ref<TextureCube>& texture) const;
        VkDescriptorImageInfo GetImageInfo(const Ref<Image2D>& image) const;

    private:
        uint32_t m_SetCount = 0;
        uint32_t m_FramesInFlight = 0;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        std::vector<SetState> m_Sets;
    };
}
