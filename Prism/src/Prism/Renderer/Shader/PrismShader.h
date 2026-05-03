#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Renderer/Shader.h"
#include "ShaderPropertyDeclaration.h"
#include "ShaderCommand.h"
#include "ShaderVariant.h"
#include "Prism/Renderer/Shader/Parser/ShaderParserData.h"
#include <functional>
#include <unordered_map>

namespace Prism
{
	using ShaderReloadedCallback = std::function<void()>;

	class PRISM_API PrismShader : public RefCounted
	{
	public:
		static Ref<PrismShader> Create(const std::string& path);
		static Ref<PrismShader> CreateFromString(const std::string& source);

		Ref<Shader> GetOriginalShader() const;

	public:
		PrismShader(const std::string& path);
		PrismShader();
		~PrismShader();

		void Reload();
		void Load(const std::string& source);

	public:
		void Bind();
		void SetProperty(const Buffer& buffer);

		void AddShaderReloadedCallback(const ShaderReloadedCallback& callback);
		const std::string& GetFilePath() const { return m_FilePath; }
		const std::string& GetName() const { return m_Name; }

		const PropertyBufferDeclaration& GetDeclaration() const { return m_Declaration; }
		const Buffer& GetDefaultValueBuffer() const { return m_DefaultValueBuffer; }
		const ShaderCommand& GetShaderCommand() const { return m_ShaderCommand; }

		template<typename T>
		const T& GetDefaultValue(const std::string& name) const
		{
			const PropertyDeclaration* decl = m_Declaration.FindProperty(name);
			PR_CORE_ASSERT(decl, "Property {0} not found in shader!", name);
			return *(const T*)(m_DefaultValueBuffer.Data + decl->GetOffset());
		}
		uint32_t GetTextureSlot(const std::string& name) const;

	public: // Keyword / Variant API
		const std::vector<ShaderKeyword>& GetKeywords() const { return m_Keywords; }
		uint8_t GetKeywordIndex(const std::string& name) const;
		bool IsKeywordDefined(const std::string& name) const;
		Ref<Shader> GetVariant(KeywordMask mask) const;

	public:
		void SetMat4FromRenderThread(const std::string& name, const glm::mat4& value);
		void SetInt(const std::string& name, int value);
		void SetIntArray(const std::string& name, int* values, uint32_t size);
		void SetFloat(const std::string& name, float value);
		void SetVec3(const std::string& name, const glm::vec3& value);
		void SetVec4(const std::string& name, const glm::vec4& value);
		void SetMat4(const std::string& name, const glm::mat4& value);

	private:
		void PackDefaultValues(const std::vector<PropertyDescriptor>& properties);

	private:
		std::string m_Name;
		std::string m_FilePath;
		Ref<Shader> m_Shader; // Base shader (keyword mask = 0)

		PropertyBufferDeclaration m_Declaration;
		Buffer m_DefaultValueBuffer;
		uint32_t m_NextTexSlot = 0;
		ShaderCommand m_ShaderCommand;

		std::vector<ShaderReloadedCallback> m_ReloadedCallbacks;

		// Keyword / Variant data
		std::vector<ShaderKeyword> m_Keywords;
		std::unordered_map<KeywordMask, ShaderVariant> m_Variants;

	public:
		static std::vector<Ref<PrismShader>> s_AllShaders;
	};

	class PRISM_API ShaderLibrary : public RefCounted
	{
	public:
		ShaderLibrary() = default;
		~ShaderLibrary() = default;

		void Add(const Ref<PrismShader>& shader);
		void Load(const std::string& path);
		void Load(const std::string& name, const std::string& path);
	void LoadAll(const std::string& directory);

		const Ref<PrismShader>& Get(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<PrismShader>> m_Shaders;
	};

}
