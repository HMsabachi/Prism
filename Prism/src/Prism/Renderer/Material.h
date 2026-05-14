#pragma once

#include "Shader/PrismShader.h"
#include "Shader/ShaderVariant.h"
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

		// Multi-Pass API
		uint32_t GetPassCount() const { return m_Shader->GetPassCount(); }
		void BindPass(uint32_t passIndex);

		// Keyword API
		void SetKeyword(const std::string& name, bool enabled);
		bool IsKeywordEnabled(const std::string& name) const;
		KeywordMask GetKeywordMask() const { return m_KeywordMask; }

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
		void Set(const std::string& name, const Ref<Texture2D>& texture)
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
		void Set(const std::string& name, const Ref<TextureCube>& texture)
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

		template<typename T>
		T& Get(const std::string& name)
		{
			auto decl = FindPropertyDeclaration(name);
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			return buffer.Read<T>(decl->GetOffset());
		}

		template<typename T>
		Ref<T> GetResource(const std::string& name)
		{
			auto decl = FindPropertyDeclaration(name);
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			auto& tex = *(PropertyType::Texture2D*)&buffer[decl->GetOffset()];
			uint32_t slot = tex.slot;
			PR_CORE_ASSERT(slot < m_Textures.size(), "Texture slot is invalid!");
			return m_Textures[slot];
		}

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
		Buffer m_PropertyBuffer;
		ShaderCommand m_ShaderCommand;
		std::vector<Ref<Texture>> m_Textures;
		KeywordMask m_KeywordMask = 0;
	};

	class PRISM_API MaterialInstance : public RefCounted
	{
		friend class Material;
	public:
		static Ref<MaterialInstance> Create(const Ref<Material>& material, const std::string& name = "");
	public:
		MaterialInstance(const Ref<Material>& material, const std::string& name = "");
		virtual ~MaterialInstance();

		const std::string& GetName() const { return m_Name; }

		// Multi-Pass API
		uint32_t GetPassCount() const { return m_Material->GetPassCount(); }
		void BindPass(uint32_t passIndex);

		// Keyword API
		void SetKeyword(const std::string& name, bool enabled);
		bool IsKeywordEnabled(const std::string& name) const;
		KeywordMask GetKeywordMask() const { return m_KeywordMask; }

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
		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}
		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}
		void Set(const std::string& name, const glm::mat4& value)
		{
			m_Transform = value;
		}
	#pragma endregion

		template<typename T>
		T& Get(const std::string& name)
		{
			auto decl = m_Material->FindPropertyDeclaration(name);
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			return buffer.Read<T>(decl->GetOffset());
		}

		template<typename T>
		Ref<T> GetResource(const std::string& name)
		{
			auto decl = m_Material->FindPropertyDeclaration(name);
			PR_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_PropertyBuffer;
			auto& tex = *(PropertyType::Texture2D*)&buffer[decl->GetOffset()];
			uint32_t slot = tex.slot;
			PR_CORE_ASSERT(slot < m_Textures.size(), "Texture slot is invalid!");
			return m_Textures[slot];
		}

		template<typename T>
		Ref<T> TryGetResource(const std::string& name)
		{
			auto decl = m_Material->FindPropertyDeclaration(name);
			if (!decl)
				return nullptr;
			auto& buffer = m_PropertyBuffer;
			auto& tex = *(PropertyType::Texture2D*)&buffer[decl->GetOffset()];
			uint32_t slot = tex.slot;
			if (slot >= m_Textures.size())
				return nullptr;
			return m_Textures[slot];
		}

	public:
		void Bind();
		Ref<PrismShader> GetShader() const { return m_Material->m_Shader; }
		void SetShader(const Ref<PrismShader>& shader);

	private:
		void AllocateStorage();
		void OnShaderReloaded();
		void OnMaterialValueUpdated(const PropertyDeclaration* decl);

	private:
		Ref<Material> m_Material;

		Buffer m_PropertyBuffer;
		std::vector<Ref<Texture>> m_Textures;
		std::string m_Name;

		std::unordered_set<std::string> m_OverriddenValues;
		KeywordMask m_KeywordMask = 0;
	private:
		glm::mat4 m_Transform{ 1.0f };
	};
}
