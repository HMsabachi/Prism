#include "prpch.h"
#include "VulkanComputeShader.h"

#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanPipeline.h"
#include "VulkanRenderer.h"
#include "VulkanShader.h"
#include "VulkanUniformBuffer.h"
#include "VulkanShaderStorageBuffer.h"
#include "VulkanTexture.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Shader.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"

#include <array>
#include <cstring>

namespace Prism
{
    using K = PrismShaderCompiler::CSL::ResourceKind;
    using DK = PrismShaderCompiler::DescriptorKind;

    VulkanComputeShader::VulkanComputeShader(const std::string& filePath)
        : ComputeShader(filePath)
    {
        m_Kernels.reserve(m_Compiled.Kernels.size());
        for (size_t i = 0; i < m_Compiled.Kernels.size(); ++i)
        {
            const auto& compiled = m_Compiled.Kernels[i];

            Kernel kernel;
            kernel.Name = compiled.Name;
            kernel.GroupSizeX = compiled.GroupSizeX;
            kernel.GroupSizeY = compiled.GroupSizeY;
            kernel.GroupSizeZ = compiled.GroupSizeZ;
            kernel.Shader = Shader::Create(m_Compiled, (uint32_t)i).As<VulkanShader>();

            if (!kernel.Shader)
            {
                PR_CORE_ERROR("VulkanComputeShader '{}': kernel '{}' 未产生 VulkanShader", m_Name, kernel.Name);
                m_Kernels.push_back(std::move(kernel));
                continue;
            }

            kernel.UniformData.assign(kernel.Shader->GetReflection().PushConstantSize, 0);

            for (const auto& descriptor : kernel.Shader->GetReflection().Descriptors)
            {
                if (descriptor.Set != 0)
                {
                    PR_CORE_ERROR("VulkanComputeShader '{}': 资源 '{}' 声明在 set {}，计算着色器目前只支持 set 0",
                        m_Name, descriptor.Name, descriptor.Set);
                    continue;
                }

                for (uint32_t i = 0; i < VulkanComputeShader::Kernel::MaxDispatchesPerFrame; ++i)
                {
                    VulkanDescriptorSet& set = kernel.Sets[i];

                    switch (descriptor.Kind)
                    {
                    case DK::UniformBuffer:
                        set.SetInput(descriptor.Binding, Ref<VulkanUniformBuffer>(nullptr));
                        break;
                    case DK::StorageBuffer:
                        set.SetInput(descriptor.Binding, Ref<VulkanShaderStorageBuffer>(nullptr));
                        break;
                    case DK::Sampler:
                        set.SetInput(descriptor.Binding, Ref<VulkanImage2D>(nullptr));
                        break;
                    case DK::StorageImage:
                        set.SetInput(descriptor.Binding, Ref<VulkanImage2D>(nullptr), 0);
                        break;
                    default:
                        if (i == 0)
                            PR_CORE_ERROR("VulkanComputeShader '{}': 资源 '{}' 的类型当前不支持", m_Name, descriptor.Name);
                        break;
                    }
                }
            }

            for (VulkanDescriptorSet& set : kernel.Sets)
                set.Bake();

            m_Kernels.push_back(std::move(kernel));
        }
    }

    int32_t VulkanComputeShader::FindKernel(const std::string& name) const
    {
        for (size_t i = 0; i < m_Kernels.size(); ++i)
        {
            if (m_Kernels[i].Name == name)
                return (int32_t)i;
        }
        PR_CORE_ERROR("ComputeShader '{}': 找不到名为 '{}' 的 kernel", m_Name, name);
        return -1;
    }

    bool VulkanComputeShader::HasKernel(const std::string& name) const
    {
        for (const Kernel& kernel : m_Kernels)
        {
            if (kernel.Name == name)
                return true;
        }
        return false;
    }

    void VulkanComputeShader::GetKernelThreadGroupSizes(int32_t kernel, uint32_t& x, uint32_t& y, uint32_t& z) const
    {
        if (!IsLegalKernel(kernel))
            return;
        x = m_Kernels[kernel].GroupSizeX;
        y = m_Kernels[kernel].GroupSizeY;
        z = m_Kernels[kernel].GroupSizeZ;
    }

    bool VulkanComputeShader::IsLegalKernel(int32_t kernel) const
    {
        if (kernel < 0 || kernel >= (int32_t)m_Kernels.size())
        {
            PR_CORE_ERROR("ComputeShader '{}': 不合法的 Kernel ID {}", m_Name, kernel);
            return false;
        }
        return true;
    }

    void VulkanComputeShader::SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::UniformBuffer);
        if (slot < 0 || !ubo)
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([self, kernelIndex, binding, ubo]() mutable
        {
            for (VulkanDescriptorSet& set : self->m_Kernels[kernelIndex].Sets)
                set.SetInput(binding, ubo.As<VulkanUniformBuffer>());
        });
    }

    void VulkanComputeShader::SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::StorageBuffer);
        if (slot < 0 || !ssbo)
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([self, kernelIndex, binding, ssbo]() mutable
        {
            for (VulkanDescriptorSet& set : self->m_Kernels[kernelIndex].Sets)
                set.SetInput(binding, ssbo.As<VulkanShaderStorageBuffer>());
        });
    }

    void VulkanComputeShader::SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::Sampler2D);
        if (slot < 0 || !image)
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([self, kernelIndex, binding, image]() mutable
        {
            for (VulkanDescriptorSet& set : self->m_Kernels[kernelIndex].Sets)
                set.SetInput(binding, image.As<VulkanImage2D>());
        });
    }

    void VulkanComputeShader::SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::SamplerCube);
        if (slot < 0 || !image)
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([self, kernelIndex, binding, image]() mutable
        {
            for (VulkanDescriptorSet& set : self->m_Kernels[kernelIndex].Sets)
                set.SetInput(binding, image.As<VulkanImageCube>());
        });
    }

    void VulkanComputeShader::SetTexture2D(int32_t kernel, const std::string& name, Ref<Texture2D> texture)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::Sampler2D);
        if (slot < 0 || !texture)
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([self, kernelIndex, binding, texture]() mutable
        {
            for (VulkanDescriptorSet& set : self->m_Kernels[kernelIndex].Sets)
                set.SetInput(binding, texture.As<VulkanTexture2D>());
        });
    }

    void VulkanComputeShader::SetTextureCube(int32_t kernel, const std::string& name, Ref<TextureCube> texture)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::SamplerCube);
        if (slot < 0 || !texture)
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([self, kernelIndex, binding, texture]() mutable
        {
            for (VulkanDescriptorSet& set : self->m_Kernels[kernelIndex].Sets)
                set.SetInput(binding, texture.As<VulkanTextureCube>());
        });
    }

    void VulkanComputeShader::SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::Image2D);
        if (slot < 0 || !image)
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([self, kernelIndex, binding, level, image]() mutable
        {
            for (VulkanDescriptorSet& set : self->m_Kernels[kernelIndex].Sets)
                set.SetInput(binding, image.As<VulkanImage2D>(), level);
        });
    }

    void VulkanComputeShader::SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::ImageCube);
        if (slot < 0 || !image)
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([self, kernelIndex, binding, level, image]() mutable
        {
            for (VulkanDescriptorSet& set : self->m_Kernels[kernelIndex].Sets)
                set.SetInput(binding, image.As<VulkanImageCube>(), level);
        });
    }

    void VulkanComputeShader::Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ, bool force)
    {
        if (!IsLegalKernel(kernel))
            return;

        if (!m_Kernels[kernel].Shader || !m_Kernels[kernel].Sets[0].IsBaked())
            return;

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;

        Renderer::Submit([self, kernelIndex, groupsX, groupsY, groupsZ, force]() mutable
        {
            Kernel& kernelInfo = self->m_Kernels[kernelIndex];

            VulkanDescriptorSet& set = kernelInfo.Sets[kernelInfo.SetCursor];
            kernelInfo.SetCursor = (kernelInfo.SetCursor + 1) % Kernel::MaxDispatchesPerFrame;
            set.RT_Prepare();

            WeakRef<VulkanComputePipeline> pipeline =
                VulkanRenderer::GetPipelineCache().GetCompute(kernelInfo.Shader.Raw());
            VkDescriptorSet descriptorSets[] = { set.RT_GetDescriptorSet() };

            Ref<VulkanDevice> device;
            VkCommandBuffer cmdBuf = VK_NULL_HANDLE;
            if (force)
            {
                device = VulkanContext::GetCurrentDevice();
                cmdBuf = device->GetCommandBuffer(true);
            }
            else
            {
                cmdBuf = VulkanRenderer::RT_GetActiveCommandBuffer();
            }

            pipeline->RT_Bind(cmdBuf, descriptorSets, 1);

            const uint32_t uniformSize = static_cast<uint32_t>(kernelInfo.UniformData.size());
            if (uniformSize > 0)
            {
                pipeline->RT_BindPushConstant(cmdBuf, 0, uniformSize, kernelInfo.UniformData.data());
            }

            pipeline->RT_Dispatch(cmdBuf, groupsX, groupsY, groupsZ);

            if (device)
                device->FlushCommandBuffer(cmdBuf);
        });
    }

    void VulkanComputeShader::WriteUniform(int32_t kernel, const std::string& name,
                                           PrismShaderCompiler::GLSLType type, const void* data, uint32_t size)
    {
        if (!IsLegalKernel(kernel))
            return;

        const int32_t index = FindUniform(name);
        if (index < 0)
            return;

        const PrismShaderCompiler::CSL::ComputeUniform& uniform = m_Compiled.Uniforms[index];

        if (uniform.Type != type || uniform.Size != size)
        {
            PR_CORE_ERROR("ComputeShader '{}': uniform '{}' 声明为 {}（{} 字节），按 {}（{} 字节）写入",
                m_Name, name, PrismShaderCompiler::GLSLTypeUtil::ToString(uniform.Type), uniform.Size,
                PrismShaderCompiler::GLSLTypeUtil::ToString(type), size);
            return;
        }

        std::array<uint8_t, 16> bytes{};
        std::memcpy(bytes.data(), data, size);

        Ref<VulkanComputeShader> self = this;
        uint32_t kernelIndex = (uint32_t)kernel;
        uint32_t offset = uniform.Offset;

        Renderer::Submit([self, kernelIndex, offset, bytes, size]() mutable
        {
            std::memcpy(self->m_Kernels[kernelIndex].UniformData.data() + offset, bytes.data(), size);
        });
    }

    void VulkanComputeShader::SetBool(int32_t kernel, const std::string& name, bool value)
    {
        const int32_t packed = value ? 1 : 0;
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Bool, &packed, sizeof(packed));
    }

    void VulkanComputeShader::SetInt(int32_t kernel, const std::string& name, int32_t value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Int, &value, sizeof(value));
    }

    void VulkanComputeShader::SetUInt(int32_t kernel, const std::string& name, uint32_t value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::UInt, &value, sizeof(value));
    }

    void VulkanComputeShader::SetFloat(int32_t kernel, const std::string& name, float value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Float, &value, sizeof(value));
    }

    void VulkanComputeShader::SetVector2(int32_t kernel, const std::string& name, const glm::vec2& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Vec2, &value.x, 2 * sizeof(float));
    }

    void VulkanComputeShader::SetVector3(int32_t kernel, const std::string& name, const glm::vec3& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Vec3, &value.x, 3 * sizeof(float));
    }

    void VulkanComputeShader::SetVector4(int32_t kernel, const std::string& name, const glm::vec4& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Vec4, &value.x, 4 * sizeof(float));
    }

    void VulkanComputeShader::SetIntVector2(int32_t kernel, const std::string& name, const glm::ivec2& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::IVec2, &value.x, 2 * sizeof(int32_t));
    }

    void VulkanComputeShader::SetIntVector3(int32_t kernel, const std::string& name, const glm::ivec3& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::IVec3, &value.x, 3 * sizeof(int32_t));
    }

    void VulkanComputeShader::SetIntVector4(int32_t kernel, const std::string& name, const glm::ivec4& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::IVec4, &value.x, 4 * sizeof(int32_t));
    }

    void VulkanComputeShader::SetUIntVector2(int32_t kernel, const std::string& name, const glm::uvec2& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::UVec2, &value.x, 2 * sizeof(uint32_t));
    }

    void VulkanComputeShader::SetUIntVector3(int32_t kernel, const std::string& name, const glm::uvec3& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::UVec3, &value.x, 3 * sizeof(uint32_t));
    }

    void VulkanComputeShader::SetUIntVector4(int32_t kernel, const std::string& name, const glm::uvec4& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::UVec4, &value.x, 4 * sizeof(uint32_t));
    }

    void VulkanComputeShader::SetBoolVector2(int32_t kernel, const std::string& name, const glm::bvec2& value)
    {
        const int32_t packed[2] = { value.x, value.y };
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::BVec2, packed, sizeof(packed));
    }

    void VulkanComputeShader::SetBoolVector3(int32_t kernel, const std::string& name, const glm::bvec3& value)
    {
        const int32_t packed[3] = { value.x, value.y, value.z };
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::BVec3, packed, sizeof(packed));
    }

    void VulkanComputeShader::SetBoolVector4(int32_t kernel, const std::string& name, const glm::bvec4& value)
    {
        const int32_t packed[4] = { value.x, value.y, value.z, value.w };
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::BVec4, packed, sizeof(packed));
    }

}
