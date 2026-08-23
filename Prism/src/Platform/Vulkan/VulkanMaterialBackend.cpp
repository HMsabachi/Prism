#include "prpch.h"
#include "VulkanMaterialBackend.h"

#include "Buffer/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Prism/Renderer/Texture.h"

namespace Prism
{
    VulkanMaterialBackend::VulkanMaterialBackend(const WeakRef<Material> material)
        : m_Material(material)
    {
    }

    VulkanMaterialBackend::~VulkanMaterialBackend()
    {
        auto device = VulkanContext::GetCurrentDevice();
        if (m_DescriptorPool)
            vkDestroyDescriptorPool(device->GetVulkanDevice(), m_DescriptorPool, nullptr);
        if (m_DescriptorSetLayout)
            vkDestroyDescriptorSetLayout(device->GetVulkanDevice(), m_DescriptorSetLayout, nullptr);
    }

    void VulkanMaterialBackend::OnAllocate()
    {
        auto material = m_Material.lock();
        PR_CORE_ASSERT(material);

        m_UniformBuffer = Ref<VulkanUniformBuffer>::Create((uint32_t)material->m_PropertyBuffer.GetSize());

        auto device = VulkanContext::GetCurrentDevice();
        VkDevice vkDevice = device->GetVulkanDevice();

        if (m_DescriptorPool)
        {
            vkDestroyDescriptorPool(vkDevice, m_DescriptorPool, nullptr);
            m_DescriptorPool = VK_NULL_HANDLE;
        }
        if (m_DescriptorSetLayout)
        {
            vkDestroyDescriptorSetLayout(vkDevice, m_DescriptorSetLayout, nullptr);
            m_DescriptorSetLayout = VK_NULL_HANDLE;
        }

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        VkDescriptorSetLayoutBinding uboBinding{};
        uboBinding.binding = 0;
        uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBinding.descriptorCount = 1;
        uboBinding.stageFlags = VK_SHADER_STAGE_ALL;
        bindings.push_back(uboBinding);

        for (auto& texturePair : material->m_Textures)
        {
            VkDescriptorSetLayoutBinding textureBinding{};
            textureBinding.binding = texturePair.first + 1;
            textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureBinding.descriptorCount = 1;
            textureBinding.stageFlags = VK_SHADER_STAGE_ALL;
            bindings.push_back(textureBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = (uint32_t)bindings.size();
        layoutInfo.pBindings = bindings.data();
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &m_DescriptorSetLayout));

        std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanFramesInFlight });
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)material->m_Textures.size() * VulkanFramesInFlight });

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = VulkanFramesInFlight;
        VK_CHECK_RESULT(vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &m_DescriptorPool));

        std::array<VkDescriptorSetLayout, VulkanFramesInFlight> layouts{};
        layouts.fill(m_DescriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = VulkanFramesInFlight;
        allocInfo.pSetLayouts = layouts.data();
        VK_CHECK_RESULT(vkAllocateDescriptorSets(vkDevice, &allocInfo, m_DescriptorSets));

        material->m_DataDirty = true;
        material->m_TexturesDirty = true;
    }

    VkDescriptorSet VulkanMaterialBackend::RT_GetDescriptorSet() const
    {
        auto material = m_Material.lock();
        PR_CORE_ASSERT(material);

        auto device = VulkanContext::GetCurrentDevice();
        VkDevice vkDevice = device->GetVulkanDevice();

        if (material->m_DataDirty)
        {
            VkDescriptorBufferInfo bufferInfo = m_UniformBuffer->GetDescriptorBufferInfo();
            std::array<VkWriteDescriptorSet, VulkanFramesInFlight> writes{};
            for (uint32_t i = 0; i < VulkanFramesInFlight; i++)
            {
                writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[i].dstSet = m_DescriptorSets[i];
                writes[i].dstBinding = 0;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writes[i].descriptorCount = 1;
                writes[i].pBufferInfo = &bufferInfo;
            }

            vkUpdateDescriptorSets(vkDevice, VulkanFramesInFlight, writes.data(), 0, nullptr);
            material->m_DataDirty = false;
        }

        if (material->m_TexturesDirty)
        {
            std::vector<VkDescriptorImageInfo> imageInfos;
            std::vector<VkWriteDescriptorSet> writes;
            imageInfos.reserve(material->m_Textures.size());
            writes.reserve(material->m_Textures.size() * VulkanFramesInFlight);

            for (auto& texturePair : material->m_Textures)
            {
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                if (texturePair.second)
                {
                    if (texturePair.second->GetType() == TextureType::Texture2D)
                        imageInfo = texturePair.second.As<VulkanTexture2D>()->GetVulkanDescriptorInfo();
                    else if (texturePair.second->GetType() == TextureType::TextureCube)
                        imageInfo = texturePair.second.As<VulkanTextureCube>()->GetVulkanDescriptorInfo();
                }

                imageInfos.push_back(imageInfo);
                for (uint32_t i = 0; i < VulkanFramesInFlight; i++)
                {
                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = m_DescriptorSets[i];
                    write.dstBinding = texturePair.first + 1;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    write.descriptorCount = 1;
                    write.pImageInfo = &imageInfos.back();
                    writes.push_back(write);
                }
            }

            if (!writes.empty())
                vkUpdateDescriptorSets(vkDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);
            material->m_TexturesDirty = false;
        }

        return m_DescriptorSets[0];
    }
}
