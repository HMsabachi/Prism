#include "prpch.h"
#include "Material.h"


namespace Prism
{

	// //////////////////////////////////////////////////////
	// -----------------------Material-----------------------
	// ///////////////////////////////////////////////////////
	Prism::Ref<Prism::Material> Material::Create(const Ref<PrismShader>& shader)
	{
		return CreateRef<Material>(shader);
	}
	Material::Material(const Ref<PrismShader>& shader)
		: m_Shader(shader), m_ShaderProperty(shader->GetProperty())
	{
		m_Shader->AddShaderReloadedCallback(std::bind(&Material::OnShaderReloaded, this));
		AllocateStorage();
	}
	Material::~Material()
	{

	}
	void Material::AllocateStorage()
	{
		const auto& propertyBuffer = m_Shader->GetProperty().GetDefaultValueBuffer();
		m_PropertyBuffer = propertyBuffer.Copy();
		InitTextures();
	}
	void Material::InitTextures()
	{
		for(const auto& decl : m_ShaderProperty.GetDeclaration())
		{
			if (decl->GetType() == PropertyDeclaration::Type::Texture2D ||
				decl->GetType() == PropertyDeclaration::Type::TextureCube)
			{
				uint32_t slot = *(PropertyType::Texture2D*)&m_PropertyBuffer[decl->GetOffset()];
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
	void Material::Bind() const
	{
		m_Shader->Bind();
		if (m_PropertyBuffer)
			m_Shader->SetProperty(m_PropertyBuffer);
		BindTextures();
	}
	void Material::BindTextures() const
	{
		for (size_t i = 0; i < m_Textures.size(); i++)
		{
			auto& texture = m_Textures[i];
			if (texture)
			{
				texture->Bind(i);
				//PR_CORE_INFO("Bind Texture {0} to slot {1}", texture->GetRendererID(), i);
			}
				
		}
	}
	const Prism::PropertyDeclaration* Material::FindPropertyDeclaration(const std::string& name) const
	{
		return m_ShaderProperty.GetDeclaration().FindUniform(name);
	}


	// //////////////////////////////////////////////////////////////
	// -----------------------MaterialInstance-----------------------
	// //////////////////////////////////////////////////////////////
	Prism::Ref<Prism::MaterialInstance> MaterialInstance::Create(const Ref<Material>& material)
	{
		return CreateRef<MaterialInstance>(material);
	}
	MaterialInstance::MaterialInstance(const Ref<Material>& material)
		: m_Material(material)
	{
		m_Material->m_MaterialInstances.insert(this);
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
		const auto& propertyBuffer = m_Material->m_Shader->GetProperty().GetDefaultValueBuffer();
		m_PropertyBuffer = propertyBuffer.Copy();
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
	void MaterialInstance::Bind() const
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