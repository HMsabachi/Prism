#pragma once

#include "Prism/Renderer/Shader.h"
#include "Vulkan.h"

#include "Prism/Utilities/StaticVector.h"

#include <PrismShaderCore/Generator/ReflectionGenerator.h>

#include <span>
#include <vector>

namespace Prism
{
    class VulkanShader : public Shader
    {
    public:
        VulkanShader(std::span<const uint8_t> spirvVertex, std::span<const uint8_t> spirvFragment, std::span<const PrismShaderCompiler::DescriptorInfo> reflection);
        VulkanShader(std::span<const uint8_t> spirvCompute, std::span<const PrismShaderCompiler::DescriptorInfo> reflection);
        virtual ~VulkanShader();

        const StaticVector<VkPipelineShaderStageCreateInfo, 2>& GetPipelineShaderStageCreateInfos() const { return m_ShaderStages; }
        VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
        const PrismShaderCompiler::PassReflection& GetReflection() const { return m_Reflection; }
        bool IsCompute() const { return m_IsCompute; }
    private:
        void CreateShaderStage(VkShaderStageFlagBits stage, std::span<const uint8_t> code);
        void CreateDescriptorSetLayouts();
        void CreatePipelineLayout();
    private:
        StaticVector<VkPipelineShaderStageCreateInfo, 2> m_ShaderStages;
        StaticVector<VkShaderModule, 2> m_ShaderModules;
        std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        PrismShaderCompiler::PassReflection m_Reflection;
        bool m_IsCompute = false;
    };
}
