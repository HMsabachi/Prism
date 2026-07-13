#include "prpch.h"
#include "../Shader.h"
#include "ComputeShader.h"

#include "Prism/ShaderCompiler/ShaderCompiler.h"

#include "../Texture.h"
#include "../Renderer.h"
#include "../Buffer/ShaderStorageBuffer.h"


namespace Prism
{

	std::vector<Ref<ComputeShader>> ComputeShader::s_AllComputeShader;

	Ref<ComputeShader> ComputeShader::Create(const std::string& filePath)
	{
		auto shader = Ref<ComputeShader>::Create(filePath);
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
			Resource r;
			r.kind = resource.Kind;
			r.readOnly = resource.ReadOnly;
			r.writeOnly = resource.WriteOnly;
			r.name = resource.Name;
			r.binding = resource.Binding;
			m_ResourcesMap[r.name] = index++;
			m_Resources.push_back(std::move(r));
		}

		for (uint32_t i = 0; i < m_Compiled.Kernels.size(); ++i)
		{
			auto out = compiler.GenerateComputeGLSL(m_Compiled, i);

			for (auto& w : out.Warnings)
				PR_CORE_WARN("CSL kernel '{}' GLSL: {}", m_Compiled.Kernels[i].Name, w);
			for (auto& e : out.Errors)
				PR_CORE_ERROR("CSL kernel '{}' GLSL: {}", m_Compiled.Kernels[i].Name, e);

			Kernel k;
			k.name = m_Compiled.Kernels[i].Name;
			k.groupSizeX = m_Compiled.Kernels[i].GroupSizeX;
			k.groupSizeY = m_Compiled.Kernels[i].GroupSizeY;
			k.groupSizeZ = m_Compiled.Kernels[i].GroupSizeZ;

			if (out.Errors.empty() && !out.Source.empty())
				k.shader.Reset(Shader::Create(out.Source));
			else
				PR_CORE_ERROR("ComputeShader::Load - kernel '{}' produced no GLSL, skipped", k.name);

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

	static TextureAccess GetTextureAccess(bool readOnly, bool writeOnly)
	{
		if (readOnly && !writeOnly)  return TextureAccess::ReadOnly;
		if (writeOnly && !readOnly)  return TextureAccess::WriteOnly;
		return TextureAccess::ReadWrite;
	}

	static bool IsImage(PrismShaderCompiler::CSL::ResourceKind kind)
	{
		using K = PrismShaderCompiler::CSL::ResourceKind;
		return kind == K::Image2D || kind == K::Image3D || kind == K::ImageCube;
	}

	void ComputeShader::Dispatch(int32_t kernel, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
	{
		if (!IsLegalID(kernel)) return;
		std::unordered_set<Ref<Texture>> usedTextures;
		std::unordered_set<Ref<ShaderStorageBuffer>> usedBuffer;
		auto& k = m_Kernels[kernel];
		k.shader->Bind();
		for (auto& res : m_Resources)
		{
			if (auto ssbo = res.ssbo)
			{
				usedBuffer.insert(ssbo);
				ssbo->Bind(res.binding);
			}
			if (auto texture = res.texture2D)
			{
				usedTextures.insert(texture);
				if (IsImage(res.kind))
					texture->BindImage(res.binding, GetTextureAccess(res.readOnly, res.writeOnly), res.layered, res.level);
				else
					texture->Bind(res.binding);
			}
			if (auto textureCube = res.textureCube)
			{
				usedTextures.insert(textureCube);
				if (IsImage(res.kind))
					textureCube->BindImage(res.binding, GetTextureAccess(res.readOnly, res.writeOnly), res.layered, res.level);
				else
					textureCube->Bind(res.binding);
			}
		}
		k.shader->DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
	}

}