#include "prpch.h"
#include "Material.h"

namespace Prism
{

	// //////////////////////////////////////////////////////
	// -----------------------Material-----------------------
	// ///////////////////////////////////////////////////////
	Ref<Material> Material::Create(const Ref<PrismShader>& shader)
	{
		return Ref<Material>::Create(shader);
	}

	Material::Material(const Ref<PrismShader>& shader)
		: m_Shader(shader), m_ShaderCommand(shader->GetShaderCommand())
	{
		m_Shader->AddShaderReloadedCallback(std::bind(&Material::OnShaderReloaded, this));
		AllocateStorage();
	}

	Material::~Material()
	{
	}

	void Material::AllocateStorage()
	{
		const auto& defaultBuffer = m_Shader->GetDefaultValueBuffer();
		m_PropertyBuffer = defaultBuffer.Copy();
		InitTextures();
	}

	void Material::InitTextures()
	{
		for (const auto& decl : m_Shader->GetDeclaration())
		{
			if (decl.GetType() == PropertyDeclarationType::Texture2D ||
				decl.GetType() == PropertyDeclarationType::TextureCube)
			{
				uint32_t slot = *(PropertyType::Texture2D*)&m_PropertyBuffer[decl.GetOffset()];
				if (m_Textures.size() <= slot)
					m_Textures.resize((size_t)slot + 1);
			}
		}
	}

	void Material::OnShaderReloaded()
	{
		AllocateStorage();
		for (auto mi : m_MaterialInstances)
			mi->OnShaderReloaded();
	}

	void Material::Bind()
	{
		m_Shader->Bind();
		if (m_PropertyBuffer)
			m_Shader->SetProperty(m_PropertyBuffer);
		BindTextures();
	}

	void Material::BindTextures()
	{
		for (size_t i = 0; i < m_Textures.size(); i++)
		{
			auto& texture = m_Textures[i];
			if (texture)
				texture->Bind(i);
		}
	}

	void Material::SetKeyword(const std::string& name, bool enabled)
	{
		if (!m_Shader->IsKeywordDefined(name))
		{
			PR_CORE_WARN("Keyword '{0}' is not defined in shader '{1}'", name, m_Shader->GetName());
			return;
		}
		uint8_t index = m_Shader->GetKeywordIndex(name);
		if (enabled)
			m_KeywordMask |= (KeywordMask(1) << index);
		else
			m_KeywordMask &= ~(KeywordMask(1) << index);
	}

	bool Material::IsKeywordEnabled(const std::string& name) const
	{
		if (!m_Shader->IsKeywordDefined(name))
			return false;
		uint8_t index = m_Shader->GetKeywordIndex(name);
		return (m_KeywordMask & (KeywordMask(1) << index)) != 0;
	}

	const PropertyDeclaration* Material::FindPropertyDeclaration(const std::string& name) const
	{
		return m_Shader->GetDeclaration().FindProperty(name);
	}


	// //////////////////////////////////////////////////////////////
	// -----------------------MaterialInstance-----------------------
	// //////////////////////////////////////////////////////////////
	Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& material, const std::string& name)
	{
		return Ref<MaterialInstance>::Create(material, name);
	}

	MaterialInstance::MaterialInstance(const Ref<Material>& material, const std::string& name)
		: m_Material(material), m_Name(name)
	{
		m_Material->m_MaterialInstances.insert(this);
		m_KeywordMask = m_Material->m_KeywordMask; // Inherit keyword state
		AllocateStorage();
	}

	MaterialInstance::~MaterialInstance()
	{
		m_Material->m_MaterialInstances.erase(this);
	}

	void MaterialInstance::OnShaderReloaded()
	{
		AllocateStorage();
		m_OverriddenValues.clear();
	}

	void MaterialInstance::AllocateStorage()
	{
		const auto& defaultBuffer = m_Material->m_Shader->GetDefaultValueBuffer();
		m_PropertyBuffer = defaultBuffer.Copy();
	}

	void MaterialInstance::OnMaterialValueUpdated(const PropertyDeclaration* decl)
	{
		if (m_OverriddenValues.find(decl->GetName()) == m_OverriddenValues.end())
		{
			auto& buffer = m_PropertyBuffer;
			auto& materialBuffer = m_Material->GetPropertyBuffer();
			buffer.Write(materialBuffer.Data + decl->GetOffset(), decl->GetSize(), decl->GetOffset());
		}
	}

	void MaterialInstance::SetKeyword(const std::string& name, bool enabled)
	{
		auto shader = m_Material->m_Shader;
		if (!shader->IsKeywordDefined(name))
		{
			PR_CORE_WARN("Keyword '{0}' is not defined in shader '{1}'", name, shader->GetName());
			return;
		}
		uint8_t index = shader->GetKeywordIndex(name);
		if (enabled)
			m_KeywordMask |= (KeywordMask(1) << index);
		else
			m_KeywordMask &= ~(KeywordMask(1) << index);
	}

	bool MaterialInstance::IsKeywordEnabled(const std::string& name) const
	{
		auto shader = m_Material->m_Shader;
		if (!shader->IsKeywordDefined(name))
			return false;
		uint8_t index = shader->GetKeywordIndex(name);
		return (m_KeywordMask & (KeywordMask(1) << index)) != 0;
	}

	void MaterialInstance::Bind()
	{
		m_Material->m_Shader->Bind();
		if (m_PropertyBuffer)
			m_Material->m_Shader->SetProperty(m_PropertyBuffer);

		m_Material->BindTextures();
		for (size_t i = 0; i < m_Textures.size(); i++)
		{
			auto& texture = m_Textures[i];
			if (texture)
				texture->Bind(i);
		}
		m_Material->m_Shader->GetOriginalShader()->SetMat4("Prism_Model", m_Transform);
	}
}
