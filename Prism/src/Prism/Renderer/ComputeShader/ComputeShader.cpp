#include "prpch.h"
#include "../Shader.h"
#include "ComputeShader.h"
#include "ComputeShaderParserData.h"
#include "ComputeShaderParser.h"

#include "../Buffer/ShaderStorageBuffer.h"
#include "../Texture.h"


namespace Prism
{
	std::vector<Ref<ComputeShader>> ComputeShader::s_AllComputeShader;

	Ref<ComputeShader> ComputeShader::Create(const std::string& filePath)
	{
		auto shader = CreateRef<ComputeShader>(filePath);
		s_AllComputeShader.push_back(shader);
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
		auto source = File::ReadFile(m_FilePath);
		auto result = ComputeShaderParser::Parse(source);
		uint32_t index = 0;
		for (auto& resource : result.resources)
		{
			Resource r;
			r.name = resource.name;
			r.type = resource.type;
			r.binding = resource.binding;
			m_ResourcesMap[r.name] = index++;
			m_Resources.push_back(r);
		}
		for (auto& kernel : result.kernels)
		{
			Kernel k;
			k.name = kernel.name;
			k.groupSizeX = kernel.numThreads[0];
			k.groupSizeY = kernel.numThreads[1];
			k.groupSizeZ = kernel.numThreads[2];
			k.shader.reset(Shader::Create(m_Name, kernel.source));
			m_Kernels.push_back(k);
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

	void ComputeShader::SetBuffer(int32_t kernel,const std::string& name, Ref<ShaderStorageBuffer>& ssbo)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].ssbo = ssbo;
	}
	void ComputeShader::SetImage2D(int32_t kernel,const std::string& name, Ref<Texture2D>& tex, uint32_t level, bool layered)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].texture2D = tex;
		m_Resources[id].level = level;
		m_Resources[id].layered = layered;
	}
	void ComputeShader::SetImageCube(int32_t kernel, const std::string& name, Ref<TextureCube>& tex, uint32_t level, bool layered)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].textureCube = tex;
		m_Resources[id].level = level;
		m_Resources[id].layered = layered;
	}
	void ComputeShader::SetTexture2D(int32_t kernel, const std::string& name, Ref<Texture2D>& tex)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].texture2D = tex;
	}
	void ComputeShader::SetTextureCube(int32_t kernel, const std::string& name, Ref<TextureCube>& tex)
	{
		auto id = FindRes(name);
		if (id == -1) return;
		m_Resources[id].textureCube = tex;
	}

	void ComputeShader::SetInt(int32_t kernel, const std::string& name, int32_t value)
	{
		if(!IsLegalID(kernel)) return;
		auto& k = m_Kernels[kernel];
		k.shader->Bind();
		k.shader->SetInt(name, value);
	}

	void ComputeShader::SetFloat(int32_t kernel, const std::string& name, float value)
	{
		if (!IsLegalID(kernel)) return;
		auto& k = m_Kernels[kernel];
		k.shader->Bind();
		k.shader->SetFloat(name, value);
	}

	static TextureAccess GetTextureAccess(ComputeShaderResourceType type)
	{
		switch (type)
		{
		case ComputeShaderResourceType::None: return TextureAccess::ReadOnly;
		case ComputeShaderResourceType::RBuffer: return TextureAccess::ReadOnly;
		case ComputeShaderResourceType::WBuffer: return TextureAccess::WriteOnly;
		case ComputeShaderResourceType::RWBuffer: return TextureAccess::ReadWrite;
		case ComputeShaderResourceType::RImage2D: return TextureAccess::ReadOnly;
		case ComputeShaderResourceType::WImage2D: return TextureAccess::WriteOnly;
		case ComputeShaderResourceType::RWImage2D: return TextureAccess::ReadWrite;
		case ComputeShaderResourceType::RImageCube: return TextureAccess::ReadOnly;
		case ComputeShaderResourceType::WImageCube: return TextureAccess::WriteOnly;
		case ComputeShaderResourceType::RWImageCube: return TextureAccess::ReadWrite;
		case ComputeShaderResourceType::Texture2D: return TextureAccess::ReadOnly;
		case ComputeShaderResourceType::TextureCube: return TextureAccess::ReadOnly;
		default: return TextureAccess::ReadOnly;
		}
	}

	static bool IsImage(ComputeShaderResourceType type)
	{
		switch (type)
		{
		case ComputeShaderResourceType::RImage2D:
		case ComputeShaderResourceType::WImage2D:
		case ComputeShaderResourceType::RWImage2D:
		case ComputeShaderResourceType::RImageCube:
		case ComputeShaderResourceType::WImageCube:
		case ComputeShaderResourceType::RWImageCube:
			return true;
		default:
			return false;
		}
	}

	void ComputeShader::Dispatch(int32_t kernel, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
	{
		if (!IsLegalID(kernel)) return;
		auto& k = m_Kernels[kernel];
		k.shader->Bind();
		for (auto& res : m_Resources)
		{
			if (auto ssbo = res.ssbo.lock())
				ssbo->Bind(res.binding);
			if (auto texture = res.texture2D.lock())
			{
				if (IsImage(res.type))
					texture->BindImage(res.binding, GetTextureAccess(res.type), res.layered, res.level);
				else
					texture->Bind(res.binding);
			}
			if (auto textureCube = res.textureCube.lock())
			{
				if (IsImage(res.type))
					textureCube->BindImage(res.binding, GetTextureAccess(res.type), res.layered, res.level);
				else
					textureCube->Bind(res.binding);
			}
		}
		k.shader->DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
	}
}