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

    namespace
    {
        struct DispatchBinding
        {
            uint32_t Binding = 0;
            K Kind = K::StorageBuffer;
            uint32_t Level = 0;
            Ref<RefCounted> Resource;
        };

        void ApplySlot(VulkanDescriptorSet& set, const DispatchBinding& entry)
        {
            switch (entry.Kind)
            {
            case K::UniformBuffer:
                set.SetInput(entry.Binding, entry.Resource.As<VulkanUniformBuffer>());
                break;
            case K::StorageBuffer:
                set.SetInput(entry.Binding, entry.Resource.As<VulkanShaderStorageBuffer>());
                break;
            case K::Sampler2D:
                set.SetInput(entry.Binding, entry.Resource.As<VulkanImage2D>());
                break;
            case K::SamplerCube:
                set.SetInput(entry.Binding, entry.Resource.As<VulkanImageCube>());
                break;
            case K::Image2D:
                set.SetInput(entry.Binding, entry.Resource.As<VulkanImage2D>(), entry.Level);
                break;
            case K::ImageCube:
                set.SetInput(entry.Binding, entry.Resource.As<VulkanImageCube>(), entry.Level);
                break;
            default:
                PR_CORE_ASSERT(false, "VulkanComputeShader: 不支持的资源类型");
                break;
            }
        }
    }

    VulkanComputeShader::VulkanComputeShader(const std::string& filePath)
        : ComputeShader(filePath)
    {
        m_KernelValues.resize(m_Kernels.size());
        for (auto& values : m_KernelValues)
            values.Slots.resize(m_Slots.size());

        m_KernelResources.reserve(m_Kernels.size());
        for (size_t i = 0; i < m_Kernels.size(); ++i)
        {
            Ref<KernelResources> resources = new KernelResources();
            resources->Shader = m_KernelShaders[i].As<VulkanShader>();

            if (!resources->Shader)
            {
                PR_CORE_ERROR("VulkanComputeShader '{}': kernel '{}' 未产生 VulkanShader", m_Name, m_Kernels[i].Name);
                m_KernelResources.push_back(std::move(resources));
                continue;
            }

            for (const auto& descriptor : resources->Shader->GetReflection().Descriptors)
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
                    resources->Set.SetInput(descriptor.Binding, Ref<VulkanUniformBuffer>(nullptr));
                    break;
                case DK::StorageBuffer:
                    resources->Set.SetInput(descriptor.Binding, Ref<VulkanShaderStorageBuffer>(nullptr));
                    break;
                case DK::Sampler:
                    resources->Set.SetInput(descriptor.Binding, Ref<VulkanImage2D>(nullptr));
                    break;
                case DK::StorageImage:
                    resources->Set.SetInput(descriptor.Binding, Ref<VulkanImage2D>(nullptr), 0);
                    break;
                default:
                    PR_CORE_ERROR("VulkanComputeShader '{}': 资源 '{}' 的类型当前不支持", m_Name, descriptor.Name);
                    break;
                }
            }

            resources->Set.Bake();
            m_KernelResources.push_back(std::move(resources));
        }
    }

    void VulkanComputeShader::SetSlot(int32_t kernel, const std::string& name, K kind,
        Ref<RefCounted> resource, uint32_t level)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, kind);
        if (slot < 0)
            return;

        SlotValue& value = m_KernelValues[kernel].Slots[slot];
        value.Resource = std::move(resource);
        value.Level = level;
    }

    void VulkanComputeShader::SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo)
    {
        SetSlot(kernel, name, K::UniformBuffer, ubo, 0);
    }

    void VulkanComputeShader::SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo)
    {
        SetSlot(kernel, name, K::StorageBuffer, ssbo, 0);
    }

    void VulkanComputeShader::SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image)
    {
        SetSlot(kernel, name, K::Sampler2D, image, 0);
    }

    void VulkanComputeShader::SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image)
    {
        SetSlot(kernel, name, K::SamplerCube, image, 0);
    }

    void VulkanComputeShader::SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level)
    {
        SetSlot(kernel, name, K::Image2D, image, level);
    }

    void VulkanComputeShader::SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level)
    {
        SetSlot(kernel, name, K::ImageCube, image, level);
    }

    void VulkanComputeShader::Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
    {
        if (!IsLegalKernel(kernel))
            return;

        Ref<KernelResources> resources = m_KernelResources[kernel];
        if (!resources->Shader || !resources->Set.IsBaked())
            return;

        const std::vector<SlotValue>& values = m_KernelValues[kernel].Slots;
        std::vector<DispatchBinding> captured;
        captured.reserve(m_Slots.size());
        for (size_t i = 0; i < m_Slots.size(); ++i)
        {
            if (!values[i].Resource)
                continue;

            captured.push_back({ m_Slots[i].Binding, m_Slots[i].Kind, values[i].Level, values[i].Resource });
        }

        Renderer::Submit([resources, captured = std::move(captured), groupsX, groupsY, groupsZ]() mutable
        {
            for (const DispatchBinding& entry : captured)
                ApplySlot(resources->Set, entry);

            resources->Set.RT_Prepare();

            WeakRef<VulkanComputePipeline> pipeline =
                VulkanRenderer::GetPipelineCache().GetCompute(resources->Shader.Raw());
            VkDescriptorSet descriptorSets[] = { resources->Set.RT_GetDescriptorSet() };
            pipeline->RT_Execute(descriptorSets, 1, groupsX, groupsY, groupsZ);
        });
    }
}
