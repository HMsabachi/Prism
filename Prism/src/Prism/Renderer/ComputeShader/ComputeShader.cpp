#include "prpch.h"
#include "ComputeShader.h"

#include "Prism/ShaderCompiler/ShaderCompiler.h"
#include "Prism/Renderer/Shader.h"
#include "Prism/Renderer/RendererAPI.h"

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

	ComputeShader::ComputeShader(const std::string& filePath)
		: m_FilePath(std::filesystem::absolute(filePath).string())
	{
		PR_PROFILE_FUNCTION();
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

		for (uint32_t i = 0; i < m_Compiled.Kernels.size(); ++i)
		{
			KernelInfo k;
			k.Name = m_Compiled.Kernels[i].Name;
			k.GroupSizeX = m_Compiled.Kernels[i].GroupSizeX;
			k.GroupSizeY = m_Compiled.Kernels[i].GroupSizeY;
			k.GroupSizeZ = m_Compiled.Kernels[i].GroupSizeZ;
			m_Kernels.push_back(std::move(k));

			m_KernelShaders.push_back(Shader::Create(m_Compiled, i));
			if (!m_KernelShaders.back())
				PR_CORE_ERROR("ComputeShader::Load - kernel '{}' produced no shader", m_Kernels.back().Name);
		}

		PR_CORE_INFO("CSL parsed '{}': {} kernels, {} resources", m_Name, m_Kernels.size(), m_Slots.size());
	}

	int32_t ComputeShader::FindKernel(const std::string& name) const
	{
		for (size_t i = 0; i < m_Kernels.size(); ++i)
		{
			if (m_Kernels[i].Name == name)
				return (int32_t)i;
		}
		PR_CORE_ERROR("ComputeShader '{}': 找不到名为 '{}' 的 kernel", m_Name, name);
		return -1;
	}

	bool ComputeShader::HasKernel(const std::string& name) const
	{
		for (const auto& kernel : m_Kernels)
		{
			if (kernel.Name == name)
				return true;
		}
		return false;
	}

	void ComputeShader::GetKernelThreadGroupSizes(int32_t kernel, uint32_t& x, uint32_t& y, uint32_t& z) const
	{
		if (!IsLegalKernel(kernel))
			return;
		x = m_Kernels[kernel].GroupSizeX;
		y = m_Kernels[kernel].GroupSizeY;
		z = m_Kernels[kernel].GroupSizeZ;
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

	bool ComputeShader::IsLegalKernel(int32_t kernel) const
	{
		if (kernel < 0 || kernel >= (int32_t)m_Kernels.size())
		{
			PR_CORE_ERROR("ComputeShader '{}': 不合法的 Kernel ID {}", m_Name, kernel);
			return false;
		}
		return true;
	}

	Ref<Shader> ComputeShader::GetKernelShader(int32_t kernel) const
	{
		if (kernel < 0 || kernel >= (int32_t)m_KernelShaders.size())
			return nullptr;
		return m_KernelShaders[kernel];
	}
}
