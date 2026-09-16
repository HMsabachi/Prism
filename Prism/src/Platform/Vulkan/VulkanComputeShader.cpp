#include "prpch.h"
#include "VulkanComputeShader.h"

#include "VulkanImage.h"
#include "VulkanPipeline.h"
#include "VulkanRenderer.h"
#include "VulkanShader.h"
#include "VulkanUniformBuffer.h"
#include "VulkanShaderStorageBuffer.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Shader.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"

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

            Ref<Kernel> kernel = Ref<Kernel>::Create();
            kernel->Name = compiled.Name;
            kernel->GroupSizeX = compiled.GroupSizeX;
            kernel->GroupSizeY = compiled.GroupSizeY;
            kernel->GroupSizeZ = compiled.GroupSizeZ;
            kernel->Shader = Shader::Create(m_Compiled, (uint32_t)i).As<VulkanShader>();

            if (!kernel->Shader)
            {
                PR_CORE_ERROR("VulkanComputeShader '{}': kernel '{}' 未产生 VulkanShader", m_Name, kernel->Name);
                m_Kernels.push_back(std::move(kernel));
                continue;
            }

            for (const auto& descriptor : kernel->Shader->GetReflection().Descriptors)
            {
                if (descriptor.Set != 0)
                {
                    PR_CORE_ERROR("VulkanComputeShader '{}': 资源 '{}' 声明在 set {}，计算着色器目前只支持 set 0",
                        m_Name, descriptor.Name, descriptor.Set);
                    continue;
                }

                switch (descriptor.Kind)
                {
                case DK::UniformBuffer:
                    kernel->Set.SetInput(descriptor.Binding, Ref<VulkanUniformBuffer>(nullptr));
                    break;
                case DK::StorageBuffer:
                    kernel->Set.SetInput(descriptor.Binding, Ref<VulkanShaderStorageBuffer>(nullptr));
                    break;
                case DK::Sampler:
                    kernel->Set.SetInput(descriptor.Binding, Ref<VulkanImage2D>(nullptr));
                    break;
                case DK::StorageImage:
                    kernel->Set.SetInput(descriptor.Binding, Ref<VulkanImage2D>(nullptr), 0);
                    break;
                default:
                    PR_CORE_ERROR("VulkanComputeShader '{}': 资源 '{}' 的类型当前不支持", m_Name, descriptor.Name);
                    break;
                }
            }

            kernel->Set.Bake();
            m_Kernels.push_back(std::move(kernel));
        }
    }

    int32_t VulkanComputeShader::FindKernel(const std::string& name) const
    {
        for (size_t i = 0; i < m_Kernels.size(); ++i)
        {
            if (m_Kernels[i]->Name == name)
                return (int32_t)i;
        }
        PR_CORE_ERROR("ComputeShader '{}': 找不到名为 '{}' 的 kernel", m_Name, name);
        return -1;
    }

    bool VulkanComputeShader::HasKernel(const std::string& name) const
    {
        for (const Ref<Kernel>& kernel : m_Kernels)
        {
            if (kernel->Name == name)
                return true;
        }
        return false;
    }

    void VulkanComputeShader::GetKernelThreadGroupSizes(int32_t kernel, uint32_t& x, uint32_t& y, uint32_t& z) const
    {
        if (!IsLegalKernel(kernel))
            return;
        x = m_Kernels[kernel]->GroupSizeX;
        y = m_Kernels[kernel]->GroupSizeY;
        z = m_Kernels[kernel]->GroupSizeZ;
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

        Ref<Kernel> kernelInfo = m_Kernels[kernel];
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([kernelInfo, binding, ubo]() mutable
        {
            kernelInfo->Set.SetInput(binding, ubo.As<VulkanUniformBuffer>());
        });
    }

    void VulkanComputeShader::SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::StorageBuffer);
        if (slot < 0 || !ssbo)
            return;

        Ref<Kernel> kernelInfo = m_Kernels[kernel];
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([kernelInfo, binding, ssbo]() mutable
        {
            kernelInfo->Set.SetInput(binding, ssbo.As<VulkanShaderStorageBuffer>());
        });
    }

    void VulkanComputeShader::SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::Sampler2D);
        if (slot < 0 || !image)
            return;

        Ref<Kernel> kernelInfo = m_Kernels[kernel];
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([kernelInfo, binding, image]() mutable
        {
            kernelInfo->Set.SetInput(binding, image.As<VulkanImage2D>());
        });
    }

    void VulkanComputeShader::SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::SamplerCube);
        if (slot < 0 || !image)
            return;

        Ref<Kernel> kernelInfo = m_Kernels[kernel];
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([kernelInfo, binding, image]() mutable
        {
            kernelInfo->Set.SetInput(binding, image.As<VulkanImageCube>());
        });
    }

    void VulkanComputeShader::SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::Image2D);
        if (slot < 0 || !image)
            return;

        Ref<Kernel> kernelInfo = m_Kernels[kernel];
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([kernelInfo, binding, level, image]() mutable
        {
            kernelInfo->Set.SetInput(binding, image.As<VulkanImage2D>(), level);
        });
    }

    void VulkanComputeShader::SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::ImageCube);
        if (slot < 0 || !image)
            return;

        Ref<Kernel> kernelInfo = m_Kernels[kernel];
        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([kernelInfo, binding, level, image]() mutable
        {
            kernelInfo->Set.SetInput(binding, image.As<VulkanImageCube>(), level);
        });
    }

    void VulkanComputeShader::Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
    {
        if (!IsLegalKernel(kernel))
            return;

        Ref<Kernel> kernelInfo = m_Kernels[kernel];
        if (!kernelInfo->Shader || !kernelInfo->Set.IsBaked())
            return;

        Renderer::Submit([kernelInfo, groupsX, groupsY, groupsZ]() mutable
        {
            kernelInfo->Set.RT_Prepare();

            WeakRef<VulkanComputePipeline> pipeline =
                VulkanRenderer::GetPipelineCache().GetCompute(kernelInfo->Shader.Raw());
            VkDescriptorSet descriptorSets[] = { kernelInfo->Set.RT_GetDescriptorSet() };
            pipeline->RT_Execute(descriptorSets, 1, groupsX, groupsY, groupsZ);
        });
    }
}
