#include "prpch.h"
#include "../Texture.h"
#include "PrismShader.h"

namespace Prism
{

	Ref<PrismShader> PrismShader::Create(const std::string& source, const bool isFile)
	{
		std::string src;
		if (isFile)
			src = ReadFile(source);
		else
			src = source;
		Ref<PrismShader> shader = CreateRef<PrismShader>(src);
		if (isFile)	shader->m_FilePath = source;
		return shader;
	}

	PrismShader::PrismShader(const std::string& source)
	{
		PrismShaderParser parser;
		m_ParseResult = parser.Parse(source);

		// 处理Property
		for (auto& property : m_ParseResult.Properties)
		{
			glm::vec4 fValue = glm::vec4(0.0f);
			glm::ivec4 iValue = glm::ivec4(0);
			switch (property.Type)
			{
			case Prism::PropertyType::Float:
			{
				ConvertValue(property.DefaultValue, property.Type, fValue);
				Ref<ShaderData::FloatPropertyElement> floatProperty = std::make_shared<ShaderData::FloatPropertyElement>(property.Name, property.DisplayName, fValue.x);
				m_Properties.Properties[property.Name] = floatProperty;
				break;
			}
			case Prism::PropertyType::Range:
			{
				ConvertValue(property.DefaultValue, property.Type, fValue);
				Ref<ShaderData::FloatPropertyElement> floatProperty = std::make_shared<ShaderData::FloatPropertyElement>(property.Name, property.DisplayName, fValue.x);
				m_Properties.Properties[property.Name] = floatProperty;
				break;
			}
			case Prism::PropertyType::Int:
			{
				ConvertValue(property.DefaultValue, property.Type, iValue);
				Ref<ShaderData::IntPropertyElement> intProperty = std::make_shared<ShaderData::IntPropertyElement>(property.Name, property.DisplayName, iValue.x);
				m_Properties.Properties[property.Name] = intProperty;
				break;
			}
			case Prism::PropertyType::Color:
			{
				ConvertValue(property.DefaultValue, property.Type, fValue);
				Ref<ShaderData::ColorPropertyElement> colorProperty = std::make_shared<ShaderData::ColorPropertyElement>(property.Name, property.DisplayName, fValue);
				m_Properties.Properties[property.Name] = colorProperty;
				break;
			}
			case Prism::PropertyType::Vector2:
			{
				ConvertValue(property.DefaultValue, property.Type, fValue);
				Ref<ShaderData::Vec2PropertyElement> vec2Property = std::make_shared<ShaderData::Vec2PropertyElement>(property.Name, property.DisplayName, glm::vec2(fValue.x, fValue.y));
				m_Properties.Properties[property.Name] = vec2Property;
				break;
			}
			case Prism::PropertyType::Vector3:
			{
				ConvertValue(property.DefaultValue, property.Type, fValue);
				Ref<ShaderData::Vec3PropertyElement> vec3Property = std::make_shared<ShaderData::Vec3PropertyElement>(property.Name, property.DisplayName, glm::vec3(fValue.x, fValue.y, fValue.z));
				m_Properties.Properties[property.Name] = vec3Property;
				break;
			}
			case Prism::PropertyType::Vector4:
			{
				ConvertValue(property.DefaultValue, property.Type, fValue);
				Ref<ShaderData::Vec4PropertyElement> vec4Property = std::make_shared<ShaderData::Vec4PropertyElement>(property.Name, property.DisplayName, glm::vec4(fValue.x, fValue.y, fValue.z, fValue.w));
				m_Properties.Properties[property.Name] = vec4Property;
				break;
			}
			case Prism::PropertyType::Texture2D:
			{
				Ref<ShaderData::Texture2DPropertyElement> texture2DProperty = std::make_shared<ShaderData::Texture2DPropertyElement>(property.Name, property.DisplayName, Texture2D::ErrorTexture);
				m_Properties.Properties[property.Name] = texture2DProperty;
				break;
			}
			}

		}
		m_Shader = Shader::Create(m_ParseResult.Passes[0].VertexShaderCode, m_ParseResult.Passes[0].FragmentShaderCode);

	}
	bool PrismShader::ConvertValue(const std::string& value, PropertyType type, glm::vec4& outValue)
	{
		if (value.empty())
		{
			outValue = glm::vec4(0.0f);
			return true;
		}
		std::istringstream iss(value);
		switch (type)
		{
		case PropertyType::Float:
		case PropertyType::Range:
		{
			float val = 0.0f;
			if (iss >> val)
			{
				outValue = glm::vec4(val);
				return true;
			}
			else
			{
				outValue = glm::vec4(0.0f);
				return false;
			}
			break;
		}
		case PropertyType::Color:
		case PropertyType::Vector4:
		{
			glm::vec4 val(0.0f);
			char c;
			if (value[0] == '(' || value[0] == '{')
			{
				std::string clean = value;
				clean.erase(std::remove_if(clean.begin(), clean.end(), [](char ch) {
					return ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ',';
					}), clean.end());
				std::istringstream vecIss(clean);
				vecIss >> val.x >> val.y >> val.z >> val.w;
				outValue = glm::vec4(val);
				return true;
			}
			outValue = glm::vec4(0.0f);
			return false;
			break;
		}
		case PropertyType::Vector3:
		{
			glm::vec4 val(0.0f);
			char c;
			if (value[0] == '(' || value[0] == '{')
			{
				std::string clean = value;
				clean.erase(std::remove_if(clean.begin(), clean.end(), [](char ch) {
					return ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ',';
					}), clean.end());

				std::istringstream vecIss(clean);
				vecIss >> val.x >> val.y >> val.z;
				outValue = glm::vec4(val);
				return true;
			}
			outValue = glm::vec4(0.0f);
			return false;
			break;
		}
		case PropertyType::Vector2:
		{
			glm::vec4 val(0.0f);
			char c;
			if (value[0] == '(' || value[0] == '{')
			{
				std::string clean = value;
				clean.erase(std::remove_if(clean.begin(), clean.end(), [](char ch) {
					return ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ',';
					}), clean.end());

				std::istringstream vecIss(clean);
				vecIss >> val.x >> val.y;
				outValue = glm::vec4(val);
				return true;
			}
			break;
		}
			
		}
	}
	bool PrismShader::ConvertValue(const std::string& value, PropertyType type, glm::ivec4& outValue)
	{
		if (value.empty())
		{
			outValue = glm::ivec4(0);
			return true;
		}
		std::istringstream iss(value);
		switch (type)
		{
		case PropertyType::Int:
		{
			float val = 0.0f;
			if (iss >> val)
			{
				outValue = glm::vec4(val);
				return true;
			}
			else
			{
				outValue = glm::vec4(0.0f);
				return false;
			}
			break;
		}
		}
	}
	std::string PrismShader::ReadFile(const std::string& filePath)
	{
		std::string result;
		std::ifstream in(filePath, std::ios::in, std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			PR_CORE_WARN("Could not open file '{0}'", filePath);
		}
		return result;
	}
	PrismShader::~PrismShader()
	{

	}
}