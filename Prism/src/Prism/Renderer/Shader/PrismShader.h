#pragma once
#if 0
#include "Prism/Core/Core.h"
#include "Prism/Renderer/Shader/Shader.h"
#include "PrismShaderParser.h"
#include "glm/glm.hpp"



namespace Prism
{
	namespace ShaderData
	{
		#pragma region Shader Property Structs Shader 属性结构体
		struct PRISM_API PropertyElement
		{
			std::string Name;
			std::string DisplayName;
			PropertyType Type;
			PropertyElement(const std::string& name, const std::string& displayName, PropertyType type)
				:Name(name), DisplayName(displayName), Type(type) {}
			virtual ~PropertyElement() = default;
			virtual void* GetDefaultValue() const = 0;
			virtual void SetValue(const void* value) = 0;
			virtual void* GetValue() const = 0;
			template<typename T>
			void operator=(T& value) { SetValue(&value); }
		};
		struct PRISM_API FloatPropertyElement : public PropertyElement
		{
			float DefaultValue;
			float Value;
			FloatPropertyElement(const std::string& name, const std::string& displayName, float defaultValue)
				: PropertyElement(name, displayName, PropertyType::Float), DefaultValue(defaultValue), Value(defaultValue) {}
			void* GetDefaultValue() const override {return(void*)(&DefaultValue); }
			void SetValue(const void* value) override {Value = *static_cast<const float*>(value);}
			void* GetValue() const override {return(void*)(&Value); }
		};
		struct PRISM_API IntPropertyElement : public PropertyElement
		{
			int DefaultValue;
			int Value;
			IntPropertyElement(const std::string& name, const std::string& displayName, int defaultValue)
				: PropertyElement(name, displayName, PropertyType::Int), DefaultValue(defaultValue), Value(defaultValue) {}
			void* GetDefaultValue() const override {return(void*)(&DefaultValue); }
			void SetValue(const void* value) override {Value = *static_cast<const int*>(value);}
			void* GetValue() const override {return(void*)(&Value); }
		};
		struct PRISM_API Vec2PropertyElement : public PropertyElement
		{
			glm::vec2 DefaultValue;
			glm::vec2 Value;
			Vec2PropertyElement(const std::string& name, const std::string& displayName, const glm::vec2& defaultValue)
				: PropertyElement(name, displayName, PropertyType::Vector2), DefaultValue(defaultValue), Value(defaultValue) {}
			void* GetDefaultValue() const override { return(void*)(&DefaultValue); }
			void SetValue(const void* value) override { Value = *static_cast<const glm::vec2*>(value); }
			void* GetValue() const override { return(void*)(&Value); }
		};
		struct PRISM_API Vec3PropertyElement : public PropertyElement
		{
			glm::vec3 DefaultValue;
			glm::vec3 Value;
			Vec3PropertyElement(const std::string& name, const std::string& displayName, const glm::vec3& defaultValue)
				: PropertyElement(name, displayName, PropertyType::Vector3), DefaultValue(defaultValue), Value(defaultValue) {}
			void* GetDefaultValue() const override { return(void*)(&DefaultValue); }
			void SetValue(const void* value) override { Value = *static_cast<const glm::vec3*>(value); }
			void* GetValue() const override { return(void*)(&Value); }
		};
		struct PRISM_API Vec4PropertyElement : public PropertyElement
		{
			glm::vec4 DefaultValue;
			glm::vec4 Value;
			Vec4PropertyElement(const std::string& name, const std::string& displayName, const glm::vec4& defaultValue)
				: PropertyElement(name, displayName, PropertyType::Vector4), DefaultValue(defaultValue), Value(defaultValue) {}
			void* GetDefaultValue() const override { return(void*)(&DefaultValue); }
			void SetValue(const void* value) override { Value = *static_cast<const glm::vec4*>(value); }
			void* GetValue() const override { return(void*)(&Value); }
		};
		struct PRISM_API ColorPropertyElement : public PropertyElement
		{
			glm::vec4 DefaultValue;
			glm::vec4 Value;
			ColorPropertyElement(const std::string& name, const std::string& displayName, const glm::vec4& defaultValue)
				: PropertyElement(name, displayName, PropertyType::Vector4), DefaultValue(defaultValue), Value(defaultValue) {
			}
			void* GetDefaultValue() const override { return(void*)(&DefaultValue); }
			void SetValue(const void* value) override { Value = *static_cast<const glm::vec4*>(value); }
			void* GetValue() const override { return(void*)(&Value); }
		};
		struct PRISM_API Texture2DPropertyElement : public PropertyElement
		{
			std::string DefaultValue;
			std::string Value;
			Texture2DPropertyElement(const std::string& name, const std::string& displayName, const std::string& defaultValue)
				: PropertyElement(name, displayName, PropertyType::Texture2D), DefaultValue(defaultValue), Value(defaultValue) {
			}
			void* GetDefaultValue() const override { return(void*)(&DefaultValue); }
			void SetValue(const void* value) override { Value = *(std::string*)(value); }
			void* GetValue() const override { return(void*)(&Value); }
		};
		#pragma endregion

		
		struct PRISM_API Property
		{
			std::unordered_map<std::string, Ref<PropertyElement>> Properties;
			template<typename T>
			T& operator[] (std::string name) const
			{
				auto it = Properties.find(name);
				if (it == Properties.end())
					PR_CORE_ASSERT(false, "Property not found!");
				return static_cast<T&>*(it->second->GetValue());
			}
			PropertyElement& operator[] (std::string name) const
			{
				auto it = Properties.find(name);
				if (it == Properties.end())
					PR_CORE_ASSERT(false, "Property not found!");
				return *(it->second);
			}

		};
		inline std::string format_as(const Property& property)
		{
			std::stringstream ss;
			ss << "\n[Prism::Property Inspector]\n";
			ss << "------------------------------------------------------------\n";
			ss << std::left << std::setw(20) << "Name"
				<< std::setw(20) << "Display Name"
				<< "Value\n";
			ss << "------------------------------------------------------------\n";

			for (const auto& [name, element] : property.Properties)
			{
				ss << std::left << std::setw(20) << name
					<< std::setw(20) << element->DisplayName << ": ";

				void* rawValue = element->GetValue();
				if (!rawValue) {
					ss << "nullptr\n";
					continue;
				}

				switch (element->Type)
				{
				case PropertyType::Float:
					ss << *static_cast<float*>(rawValue);
					break;
				case PropertyType::Int:
					ss << *static_cast<int*>(rawValue);
					break;
				case PropertyType::Vector2: {
					auto& v = *static_cast<glm::vec2*>(rawValue);
					ss << "Vec2(" << v.x << ", " << v.y << ")";
					break;
				}
				case PropertyType::Vector3: {
					auto& v = *static_cast<glm::vec3*>(rawValue);
					ss << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
					break;
				}
				case PropertyType::Vector4: {
					auto& v = *static_cast<glm::vec4*>(rawValue);
					ss << "Vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
					break;
				}
				case PropertyType::Texture2D: {
					auto& tex = *static_cast<std::string*>(rawValue);
					if (tex == "") {
						ss << "[Texture2D Object]";
					}
					else {
						ss << "[Empty Texture]";
					}
					break;
				}
				default:
					ss << "Unknown Type";
					break;
				}
				ss << "\n";
			}
			ss << "------------------------------------------------------------\n";
			return ss.str();
		}
	}
	
	class PRISM_API PrismShader
	{
	public:
		static Ref<PrismShader> Create(const std::string& source, const bool isFile = true);
		~PrismShader();

		Ref<Shader> GetOriginalShader() const { return m_Shader; }
		ShaderData::Property& GetProperties() { return m_Properties; }
	public:
		PrismShader(const std::string& source);

		std::string GetFilePath() const {return m_FilePath; };
		std::string GetName() const { return m_ParseResult.ShaderName; }
		std::string GetSource() const { return m_ParseResult.Passes[0].VertexShaderCode + m_ParseResult.Passes[0].FragmentShaderCode; }

	private:
		static bool ConvertValue(const std::string& value, PropertyType type, glm::vec4& outValue);
		static bool ConvertValue(const std::string& value, PropertyType type, glm::ivec4& outValue);
	
	private:
		Ref<Shader> m_Shader;
		static std::string ReadFile(const std::string& filePath);
	private:
		std::string m_FilePath;
		ShaderData::Property m_Properties;
		ParseResult m_ParseResult;
	};
}

#endif