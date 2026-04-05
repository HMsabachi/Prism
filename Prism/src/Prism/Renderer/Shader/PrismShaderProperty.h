#pragma once

namespace Prism
{
	class PrismShader;

	struct Texture2DType 
	{ 
		int id; 
		operator int() { return id; }
	};


	struct TextureCubeType 
	{ 
		int id; 
		operator int() { return id; }
	};



	typedef std::variant <
		std::monostate,
		float, glm::vec2, glm::vec3, glm::vec4,
		glm::mat3, glm::mat4,
		int32_t, uint32_t,
		Texture2DType, TextureCubeType
	> PropertyValue;
	namespace ShaderData
	{
		class propertyGroup;
#pragma region Shader Property Structs Shader 属性结构体
		struct PRISM_API PropertyElement
		{
			friend class PrismShader;
			friend class ShaderData::propertyGroup;
			PropertyElement() = default;
			PropertyElement(const std::string& name) : m_Name(name) {}
			const std::string& GetName() const { return m_Name; }
			const std::string& GetDisplayName() const { return m_DisplayName; }
			UniformType GetType() const { return m_Type; }
			template<typename T>
			const T& GetDefaultValue() const
			{
				const T* val = std::get_if<T>(&m_DefaultValue);
				if (!val)
				{
					PR_CORE_WARN("{0} 类型不合法", GetName());
					return T();
				}
				return *val;
			}
			const PropertyValue& GetValueVariant() const { return m_DefaultValue; }
		private:
			std::string m_Name;
			std::string m_DisplayName;
			UniformType m_Type = UniformType::None;
			PropertyValue m_DefaultValue;
		};
		struct PRISM_API propertyGroup
		{
			friend class PrismShader;
			propertyGroup() = default;

			std::unordered_map<std::string, PropertyElement>::const_iterator begin() const { return m_Properties.begin(); }
			std::unordered_map<std::string, PropertyElement>::const_iterator end() const { return m_Properties.end(); }
			const PropertyElement& operator[](const std::string& name) const
			{
				auto it = m_Properties.find(name);
				if (it == m_Properties.end())
				{
					static PropertyElement empty;
					PR_CORE_WARN("Property {0} 不存在", name);
					empty.m_Name = name;
					return empty;
				}
				return it->second;
			}
			bool empty() const { return m_Properties.empty(); }
			void AddProperty(const PropertyElement& property) { m_Properties.insert({ property.GetName(), property }); }
		private:
			std::unordered_map<std::string, PropertyElement> m_Properties;
		};
#pragma endregion
	}
}
namespace Prism::Internal {
	struct ValueToString {
		std::string operator()(std::monostate) const { return "null"; }
		std::string operator()(float v) const { return fmt::format("{:.2f}", v); }
		std::string operator()(const glm::vec2& v) const { return fmt::format("({}, {})", v.x, v.y); }
		std::string operator()(const glm::vec3& v) const { return fmt::format("({}, {}, {})", v.x, v.y, v.z); }
		std::string operator()(const glm::vec4& v) const { return fmt::format("({}, {}, {}, {})", v.x, v.y, v.z, v.w); }
		std::string operator()(const glm::mat3& v) const { return "[mat3x3]"; }
		std::string operator()(const glm::mat4& v) const { return "[mat4x4]"; }
		std::string operator()(int32_t v) const { return std::to_string(v); }
		std::string operator()(uint32_t v) const { return std::to_string(v); }
		std::string operator()(const Texture2DType& v) const { return fmt::format("Texture2D(ID:{})", v.id); }
		std::string operator()(const TextureCubeType& v) const { return fmt::format("TextureCube(ID:{})", v.id); }
	};
}