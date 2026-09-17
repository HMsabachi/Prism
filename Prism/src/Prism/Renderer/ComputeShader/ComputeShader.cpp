#include "prpch.h"
#include "ComputeShader.h"

#include "Prism/ShaderCompiler/ShaderCompiler.h"
#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Utilities/StringUtils.h"

#include "Platform/OpenGL/OpenGLComputeShader.h"
#include "Platform/Vulkan/VulkanComputeShader.h"

namespace Prism
{
    static const char* ResourceKindName(PrismShaderCompiler::CSL::ResourceKind kind)
    {
        using K = PrismShaderCompiler::CSL::ResourceKind;
        switch (kind)
        {
        case K::StorageBuffer:        return "StorageBuffer";
        case K::UniformBuffer:        return "UniformBuffer";
        case K::Sampler2D:            return "Sampler2D";
        case K::Sampler2DMS:          return "Sampler2DMS";
        case K::Sampler2DShadow:      return "Sampler2DShadow";
        case K::Sampler2DArray:       return "Sampler2DArray";
        case K::Sampler2DArrayShadow: return "Sampler2DArrayShadow";
        case K::Sampler3D:            return "Sampler3D";
        case K::SamplerCube:          return "SamplerCube";
        case K::SamplerCubeShadow:    return "SamplerCubeShadow";
        case K::Image2D:              return "Image2D";
        case K::Image3D:              return "Image3D";
        case K::ImageCube:            return "ImageCube";
        }
        return "Unknown";
    }

    Ref<ComputeShader> ComputeShader::Create(const std::string& filePath)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None:   return nullptr;
        case RendererAPIType::OpenGL: return Ref<OpenGLComputeShader>::Create(filePath);
        case RendererAPIType::Vulkan: return Ref<VulkanComputeShader>::Create(filePath);
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<ComputeShader> ComputeShader::Create(AssetHandle handle)
    {
        return AssetManager::GetAsset<ComputeShader>(handle);
    }

    ComputeShader::ComputeShader(const std::string& filePath)
        : m_FilePath(std::filesystem::absolute(filePath).string())
    {
        PR_PROFILE_FUNCTION();

        Type = AssetType::ComputeShader;
        FilePath = filePath;
        std::replace(FilePath.begin(), FilePath.end(), '\\', '/');
        FileName = Utils::RemoveExtension(Utils::GetFilename(FilePath));
        Extension = Utils::GetExtension(FilePath);
        IsDataLoaded = true;

        Load();
    }

    void ComputeShader::Load()
    {
        auto& compiler = ShaderCompiler::Get();
        m_Compiled = compiler.CompileComputeFile(m_FilePath);
        if (m_Compiled.ShaderName.empty())
        {
            PR_CORE_ERROR("ComputeShader::Load - Parse failed for '{}'", m_FilePath);
            return;
        }
        m_Name = m_Compiled.ShaderName;

        for (auto& resource : m_Compiled.Resources)
        {
            Slot slot;
            slot.Set = resource.Set;
            slot.Binding = resource.Binding;
            slot.Kind = resource.Kind;
            slot.ReadOnly = resource.ReadOnly;
            slot.WriteOnly = resource.WriteOnly;
            slot.Name = !resource.InstanceName.empty() ? resource.InstanceName
                : !resource.BlockName.empty() ? resource.BlockName
                : resource.Name;
            m_Slots.push_back(std::move(slot));
        }

        PR_CORE_INFO("CSL parsed '{}': {} kernels, {} resources", m_Name, m_Compiled.Kernels.size(), m_Slots.size());
    }

    int32_t ComputeShader::FindSlot(const std::string& name, PrismShaderCompiler::CSL::ResourceKind expected) const
    {
        for (size_t i = 0; i < m_Slots.size(); ++i)
        {
            const Slot& slot = m_Slots[i];
            if (slot.Name != name)
                continue;

            if (slot.Kind != expected)
            {
                PR_CORE_ERROR("ComputeShader '{}': 资源 '{}' 类型不符，声明为 {}，按 {} 绑定",
                    m_Name, name, ResourceKindName(slot.Kind), ResourceKindName(expected));
                return -1;
            }
            return (int32_t)i;
        }

        PR_CORE_ERROR("ComputeShader '{}': 找不到资源 '{}'", m_Name, name);
        return -1;
    }
}
