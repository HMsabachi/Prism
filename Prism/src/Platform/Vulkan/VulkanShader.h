#pragma once

#include "Prism/Renderer/Shader.h"
#include "Platform/Vulkan/Vulkan.h"

#include <PrismShaderCore/Generator/ReflectionGenerator.h>

#include <vector>

namespace Prism
{
    class VulkanShader : public Shader
    {
    public:
        VulkanShader(const std::vector<uint32_t>& spirvVertex, const std::vector<uint32_t>& spirvFragment,
            const PrismShaderCompiler::PassReflection& reflection);
        virtual ~VulkanShader();

        const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStageCreateInfos() const { return m_ShaderStages; }
        VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
        const PrismShaderCompiler::PassReflection& GetReflection() const { return m_Reflection; }
    private:
        void CreateShaderStage(VkShaderStageFlagBits stage, const std::vector<uint32_t>& code);
        void CreateDescriptorSetLayouts();
        void CreatePipelineLayout();
    private:
        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
        std::vector<VkShaderModule> m_ShaderModules;
        std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        PrismShaderCompiler::PassReflection m_Reflection;
    };
}
