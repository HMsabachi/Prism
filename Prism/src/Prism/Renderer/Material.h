#pragma once

#include "Shader/PrismShader.h"
#include <unordered_set>

namespace Prism
{
	class PRISM_API Material : public RefCounted
	{
		friend class MaterialInstance;

	public:
		static Ref<Material> Create(const Ref<PrismShader>& shader);
	public:
		Material(const Ref<PrismShader>& shader);
		virtual ~Material();

		void Bind();

		#pragma region Set函数
		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = FindPropertyDeclaration(name);
			if (!decl)
				return;
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());
			for (auto mi : m_MaterialInstances)
				mi->OnMaterialValueUpdated(decl);
		}
		template<>
		void Set<Ref<Texture2D>>(const std::string& name, const Ref<Texture2D>& texture)
		{
			auto decl = FindPropertyDeclaration(name);
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			auto& tex = *(PropertyType::Texture2D*)&buffer[decl->GetOffset()];
			uint32_t slot = tex.slot;
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}
		template<>
		void Set<Ref<TextureCube>>(const std::string& name, const Ref<TextureCube>& texture)
		{
			auto decl = FindPropertyDeclaration(name);
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			auto& tex = *(PropertyType::TextureCube*)&buffer[decl->GetOffset()];
			uint32_t slot = tex.slot;
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}
		
		#pragma endregion
	private:
		const PropertyDeclaration* FindPropertyDeclaration(const std::string& name) const;
		void AllocateStorage();
		void OnShaderReloaded();
		void BindTextures();
		const ShaderCommand& GetShaderCommand() { return m_ShaderCommand; }
		void InitTextures();
		Buffer& GetPropertyBuffer() { return m_PropertyBuffer; }
	private:
		Ref<PrismShader> m_Shader;
		std::unordered_set<MaterialInstance*> m_MaterialInstances;
		const ShaderProperty& m_ShaderProperty;
		Buffer m_PropertyBuffer;
		ShaderCommand m_ShaderCommand;

		std::vector<Ref<Texture>> m_Textures;

	};

	class PRISM_API MaterialInstance : public RefCounted
	{
		friend class Material;
	public:
		static Ref<MaterialInstance> Create(const Ref<Material>& material);
	public:
		MaterialInstance(const Ref<Material>& material);
		virtual ~MaterialInstance();

#pragma region Set函数
		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = m_Material->FindPropertyDeclaration(name);
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());

			m_OverriddenValues.insert(name);
		}
		template<>
		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto decl = m_Material->FindPropertyDeclaration(name);
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			uint32_t slot = (*(PropertyType::Texture2D*)&buffer[decl->GetOffset()]).slot;
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}
		template<>
		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}
		template<>
		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}
		template<>
		void Set(const std::string& name, const glm::mat4& value)
		{
			m_Transform = value;
		}
#pragma endregion

	public:
		void Bind();
		Ref<PrismShader> GetShader() const { return m_Material->m_Shader; }
	private:
		void AllocateStorage();
		void OnShaderReloaded();
		void OnMaterialValueUpdated(const PropertyDeclaration* decl);
	private:
		Ref<Material> m_Material;

		Buffer m_PropertyBuffer;
		std::vector<Ref<Texture>> m_Textures;

		// TODO: 这只是临时的 我需要一个更好的系统来处理材质实例覆盖的属性
		std::unordered_set<std::string> m_OverriddenValues;
	private: // TODO: 这只是临时的
		glm::mat4 m_Transform{ 1.0f };

	};
}