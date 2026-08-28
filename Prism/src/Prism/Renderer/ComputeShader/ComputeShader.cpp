#include "prpch.h"
#include "../Shader.h"
#include "ComputeShader.h"

#include "Prism/ShaderCompiler/ShaderCompiler.h"

#include "../Texture.h"
#include "../Renderer.h"
#include "../Buffer/UniformBuffer.h"
#include "../Buffer/ShaderStorageBuffer.h"


namespace Prism
{

	std::vector<Ref<ComputeShader>> ComputeShader::s_AllComputeShader;

	Ref<ComputeShader> ComputeShader::Create(const std::string& filePath)
	{
		auto shader = Ref<ComputeShader>::Create(filePath);
		// s_AllComputeShader.push_back(shader);
		return shader;
	}

	ComputeShader::ComputeShader(const std::string& filePath)
		:m_FilePath(std::filesystem::absolute(filePath).string())
	{
		PR_PROFILE_FUNCTION();
		Load();
	}

	ComputeShader::~ComputeShader()
	{

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
		m_Name = std::move(m_Compiled.ShaderName);

		PR_CORE_INFO("CSL parsed '{}': {} kernels, {} resources",
			m_Name, m_Compiled.Kernels.size(), m_Compiled.Resources.size());

		uint32_t index = 0;
		for (auto& resource : m_Compiled.Resources)
		{
			ComputeResourceBinding r;
			r.Resource = resource;

			std::string key = !resource.InstanceName.empty() ? resource.InstanceName
				: !resource.BlockName.empty() ? resource.BlockName
				: resource.Name;
			m_ResourcesMap[key] = index++;
			m_Resources.push_back(std::move(r));
		}

		for (uint32_t i = 0; i < m_Compiled.Kernels.size(); ++i)
		{
			Kernel k;
			k.name = m_Compiled.Kernels[i].Name;
			k.groupSizeX = m_Compiled.Kernels[i].GroupSizeX;
			k.groupSizeY = m_Compiled.Kernels[i].GroupSizeY;
			k.groupSizeZ = m_Compiled.Kernels[i].GroupSizeZ;

			k.shader = Shader::Create(m_Compiled, i);
			if (!k.shader)
				PR_CORE_ERROR("ComputeShader::Load - kernel '{}' produced no shader, skipped", k.name);

			m_Kernels.push_back(std::move(k));
		}
	}

	int32_t ComputeShader::FindKernel(const std::string& name)
	{
		for (int32_t i = 0; i < m_Kernels.size(); i++)
			if (m_Kernels[i].name == name) return i;
		return -1;
	}
	int32_t ComputeShader::FindRes(const std::string& name)
	{
		if (m_ResourcesMap.find(name) == m_ResourcesMap.end()) return -1;
		return m_ResourcesMap[name];
	}
	bool ComputeShader::IsLegalID(int32_t kernel)
	{
		if (kernel < 0 || kernel >= m_Kernels.size())
		{
			PR_CORE_ERROR("不合法的 Kernel ID {0}", kernel);
			return false;
		}
		return true;
	}

	void ComputeShader::SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].res = ubo;
	}
	void ComputeShader::SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].res = ssbo;
	}
	void ComputeShader::SetTexture(int32_t kernel, const std::string& name, Ref<Texture> tex)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].res = tex;
	}
	void ComputeShader::SetImage(int32_t kernel, const std::string& name, Ref<Texture> tex, uint32_t level)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].res = tex;
		m_Resources[id].Level = level;
	}

	void ComputeShader::Dispatch(int32_t kernel, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
	{
		if (!IsLegalID(kernel)) return;
		Ref<ComputeShader> instance = this;
		Renderer::GetAPI()->DispatchCompute(instance, kernel, numGroupsX, numGroupsY, numGroupsZ);
	}

	Ref<Shader> ComputeShader::GetKernelShader(int32_t kernel) const
	{
		if (kernel < 0 || kernel >= (int32_t)m_Kernels.size())
			return nullptr;
		return m_Kernels[kernel].shader;
	}

}
