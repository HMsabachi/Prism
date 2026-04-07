#include "prpch.h"
#include "PrismShaderProperty.h"


#define PR_ENABLE_LOG 1
#if PR_ENABLE_LOG
#define PR_LOG(...) PR_CORE_WARN(__VA_ARGS__)
#else
#define PR_LOG(...)
#endif
namespace Prism::ShaderData
{

	std::string PropertyElement::TypeToString(Type type)
	{
		switch (type)
		{
		case Type::Color: return "Color";
		case Type::Float: return "Float";
		case Type::Int: return "Int";
		case Type::Vector2: return "Vector2";
		case Type::Vector3: return "Vector3";
		case Type::Vector4: return "Vector4";
		case Type::Texture2D: return "Texture2D";
		case Type::TextureCube: return "TextureCube";
		}
		PR_LOG("[ShaderProperty] Unknown type: {0}", (int32_t)type);
		return "None";
	}

	Prism::ShaderData::PropertyElement::Type PropertyElement::StringToType(const std::string& type)
	{
		if(type == "Color") return Type::Color;
		if(type == "Float") return Type::Float;
		if(type == "Int") return Type::Int;
		if(type == "Vector2") return Type::Vector2;
		if(type == "Vector3") return Type::Vector3;
		if(type == "Vector4") return Type::Vector4;
		if(type == "Texture2D") return Type::Texture2D;
		if(type == "TextureCube") return Type::TextureCube;

		PR_LOG("[ShaderProperty] Unknown type: {0}", type);
		return Type::None;
	}

	Prism::OpenGLShaderUniformDeclaration::Type PropertyElement::ToUniformType(Type type)
	{
		switch (type)
		{
		case Type::Color: return OpenGLShaderUniformDeclaration::Type::VEC4;
		case Type::Float: return OpenGLShaderUniformDeclaration::Type::FLOAT32;
		case Type::Int: return OpenGLShaderUniformDeclaration::Type::INT32;
		case Type::Vector2: return OpenGLShaderUniformDeclaration::Type::VEC2;
		case Type::Vector3: return OpenGLShaderUniformDeclaration::Type::VEC3;
		case Type::Vector4: return OpenGLShaderUniformDeclaration::Type::VEC4;
		case Type::Range: return OpenGLShaderUniformDeclaration::Type::FLOAT32;
		case Type::Texture2D: return OpenGLShaderUniformDeclaration::Type::TEXTURE2D;
		case Type::TextureCube: return OpenGLShaderUniformDeclaration::Type::TEXTURECUBE;
		default:
			return OpenGLShaderUniformDeclaration::Type::NONE;
		}
	}

	const Prism::UniformData PropertyElement::GetDefaultValueAsUniform() const
	{
		return std::visit(Prism::Internal::ValueToUniform{}, m_DefaultValue);
	}

}