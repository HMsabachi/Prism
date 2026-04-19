#include "prpch.h"
#include "ShaderProperty.h"
#include "PrismShaderData.h"

#define PR_ENABLE_LOG 1
#if PR_ENABLE_LOG
	#define PR_LOG(...) PR_CORE_WARN(__VA_ARGS__)
#else
	#define PR_LOG(...)
#endif

namespace Prism
{
#pragma region 工具函数

	static PropertyDeclaration::Type ParserTypeToPropertyType(ParserPropertyType type)
	{
		switch (type)
		{
		case Prism::ParserPropertyType::Float:			return PropertyDeclaration::Type::Float;
		case Prism::ParserPropertyType::Int:			return PropertyDeclaration::Type::Int;
		case Prism::ParserPropertyType::Color:			return PropertyDeclaration::Type::Color;
		case Prism::ParserPropertyType::Vector2:		return PropertyDeclaration::Type::Vector2;
		case Prism::ParserPropertyType::Vector3:		return PropertyDeclaration::Type::Vector3;
		case Prism::ParserPropertyType::Vector4:		return PropertyDeclaration::Type::Vector4;
		case Prism::ParserPropertyType::Texture2D:		return PropertyDeclaration::Type::Texture2D;
		case Prism::ParserPropertyType::Texture2DMS:	return PropertyDeclaration::Type::Texture2DMS;
		case Prism::ParserPropertyType::TextureCube:	return PropertyDeclaration::Type::TextureCube;
		case Prism::ParserPropertyType::Range:			return PropertyDeclaration::Type::Range;
		case ParserPropertyType::Matrix3:				return PropertyDeclaration::Type::Matrix3;
		case ParserPropertyType::Matrix4:				return PropertyDeclaration::Type::Matrix4;
		default:										return PropertyDeclaration::Type::None;
		}
	}
	uint32_t PropertyDeclaration::SizeOfType(Type type)
	{
		switch (type)
		{
		case Type::Color:			return sizeof(PropertyType::Color);
		case Type::Float:			return sizeof(PropertyType::Float);
		case Type::Int:				return sizeof(PropertyType::Int);
		case Type::Vector2:			return sizeof(PropertyType::Vector2);
		case Type::Vector3:			return sizeof(PropertyType::Vector3);
		case Type::Vector4:			return sizeof(PropertyType::Vector4);
		case Type::Range:			return sizeof(PropertyType::Range);
		case Type::Texture2D:		return sizeof(PropertyType::Texture2D);
		case Type::Texture2DMS:		return sizeof(PropertyType::Texture2D);
		case Type::TextureCube:		return sizeof(PropertyType::TextureCube);
		case Type::Matrix3:			return sizeof(PropertyType::Matrix3);
		case Type::Matrix4:			return sizeof(PropertyType::Matrix4);
		default:					return 0;
		}
	}
	Type::BufferData GetDataFromPropertyType(const PropertyDeclaration& declaration, const Buffer& buffer)
	{
		Type::BufferData data;
		data.second = declaration.GetSize();
		data.first = buffer.Data + declaration.GetOffset();
		return data;
	}

#pragma endregion


#pragma region 属性类型
	namespace PropertyType
	{
		
		// -----------------Texture2D---------------------
		Texture2D::operator bool() const
		{
			return texture != nullptr;
		}
		Texture2D::operator uint32_t() const
		{
			return slot;
		}
		void Texture2D::SetTexture(Ref<Prism::Texture2D>& t)
		{
			texture = t;
		}
		std::string Texture2D::ToString() const
		{
			return fmt::format("Texture2D(Slot: {})", slot);
		}
		Texture2D::operator Ref<Prism::Texture2D>() const
		{
			return texture;
		}
		// -----------------Texture2D-----------------------
		// -----------------TextureCube---------------------
		TextureCube::operator bool() const
		{
			return texture != nullptr;
		}
		TextureCube::operator uint32_t() const
		{
			return slot;
		}
		void TextureCube::SetTexture(Ref<Prism::TextureCube>& t)
		{
			texture = t;
		}
		std::string TextureCube::ToString() const
		{
			return fmt::format("TextureCube(Slot: {})", slot);
		}
		TextureCube::operator Ref<Prism::TextureCube>() const
		{
			return texture;
		}
		// -----------------TextureCube---------------------
	}
#pragma endregion

#pragma region 描述器
	// ------------------------PropertyDeclaration-------------------------------------
	PropertyDeclaration::PropertyDeclaration(Type type, const std::string& name, const std::string displayName, uint32_t count /*= 1*/) 
		: m_Name(name), m_DisplayName(displayName), m_Count(count), m_Type(type), m_Size(SizeOfType(type)* count), m_Offset(0)
	{
	}
	// ------------------------PropertyDeclaration-------------------------------------

	// ------------------------PropertyBufferDeclaration--------------------------------
	PropertyBufferDeclaration::PropertyBufferDeclaration()
		:m_Size(0)
	{
	}
	void PropertyBufferDeclaration::Shutdown()
	{
		for (auto* property : m_Properties)
		{
			delete property;
		}
		m_Properties.clear();
	}
	void PropertyBufferDeclaration::PushUniform(PropertyDeclaration* property)
	{
		uint32_t offset = 0;
		if (m_Properties.size())
		{
			PropertyDeclaration* previous = m_Properties.back();
			offset = previous->GetOffset() + previous->GetSize();
		}
		property->SetOffset(offset);
		m_Size += property->GetSize();
		m_Properties.push_back(property);
	}
	const Prism::PropertyDeclaration* PropertyBufferDeclaration::FindUniform(const std::string& name) const
	{
		for (auto* property : m_Properties)
		{
			if (property->GetName() == name)
				return property;
		}
		PR_LOG("Property {0} not found", name);
		return nullptr;
	}
	// ------------------------PropertyBufferDeclaration--------------------------------
#pragma endregion

#pragma region 着色器属性
	// ------------------------ShaderProperty--------------------------------
	ShaderProperty::ShaderProperty()
		: m_IsLoaded(false)
	{

	}

	void ShaderProperty::Init(const std::vector<PropertyDescriptor>& properties)
	{
		m_Declaration.Shutdown();
		for (const auto& property : properties)
		{
			auto proPtr = new PropertyDeclaration(ParserTypeToPropertyType(property.Type), property.Name, property.DisplayName);
			m_Declaration.PushUniform(proPtr);
		}
		HandlePropertyValue(properties);
		m_IsLoaded = true;
	}
	const Prism::Buffer& ShaderProperty::GetDefaultValueBuffer() const
	{
		PR_CORE_ASSERT(m_IsLoaded, "ShaderProperty is not loaded! ShaderProperty未加载！");
		m_DefaultValueBuffer.SetReadOnly(true);
		return m_DefaultValueBuffer;
	}

	const Prism::PropertyBufferDeclaration& ShaderProperty::GetDeclaration() const { return m_Declaration; }

	uint32_t ShaderProperty::GetTextureSlot(const std::string& name) const
	{
		const PropertyDeclaration* decl = GetDeclaration().FindUniform(name);
		switch (decl->GetType())	
		{
		case PropertyDeclaration::Type::Texture2D:
		case PropertyDeclaration::Type::Texture2DMS:
		case PropertyDeclaration::Type::TextureCube:
			break;
		default:
			PR_CORE_ERROR("Property {0} is not a texture type!", name);
			PR_CORE_ASSERT(false, "");
		}
		return decl->GetValue<uint32_t>(m_DefaultValueBuffer);
	}

	void ShaderProperty::HandlePropertyValue(const std::vector<PropertyDescriptor>& properties)
	{
		using namespace Prism::StrParse;
		using namespace Prism::PropertyType;
		m_DefaultValueBuffer.SetReadOnly(false);
		m_DefaultValueBuffer.Allocate(m_Declaration.GetSize());

		uint32_t texIndex = 0;
		for (size_t i = 0; i < properties.size(); ++i)
		{
			const auto& property = properties[i];
			auto& value = property.DefaultValue;
			switch (property.Type)
			{
			case Prism::ParserPropertyType::Color:
			{
				Color color = Parse<Color>(value);
				m_DefaultValueBuffer.Write((byte*)&color, sizeof(Color), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::Float:
			{
				Float floatValue = Parse<Float>(value);
				m_DefaultValueBuffer.Write((byte*)&floatValue, sizeof(Float), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::Int:
			{
				Int intValue = Parse<Int>(value);
				m_DefaultValueBuffer.Write((byte*)&intValue, sizeof(Int), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::Vector2:
			{
				Vector2 vec2Value = Parse<Vector2>(value);
				m_DefaultValueBuffer.Write((byte*)&vec2Value, sizeof(Vector2), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::Vector3:
			{
				Vector3 vec3Value = Parse<Vector3>(value);
				m_DefaultValueBuffer.Write((byte*)&vec3Value, sizeof(Vector3), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::Vector4:
			{
				Vector4 vec4Value = Parse<Vector4>(value);
				m_DefaultValueBuffer.Write((byte*)&vec4Value, sizeof(Vector4), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::Texture2D:
			{
				PropertyType::Texture2D tex2d;
				tex2d.slot = texIndex++;
				m_DefaultValueBuffer.Write((byte*)&tex2d, sizeof(PropertyType::Texture2D), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::Texture2DMS:
			{
				PropertyType::Texture2D tex2d;
				tex2d.slot = texIndex++;
				m_DefaultValueBuffer.Write((byte*)&tex2d, sizeof(PropertyType::Texture2D), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::TextureCube:
			{
				PropertyType::TextureCube texCube;
				texCube.slot = texIndex++;
				m_DefaultValueBuffer.Write((byte*)&texCube, sizeof(PropertyType::TextureCube), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case Prism::ParserPropertyType::Range:
			{
				Range rangeValue;
				rangeValue.min = property.Min;
				rangeValue.max = property.Max;
				rangeValue.value = Parse<float>(value);
				m_DefaultValueBuffer.Write((byte*)&rangeValue, sizeof(Range), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case ParserPropertyType::Matrix3:
			{
				Matrix3 mat3Value = Parse<Matrix3>(value);
				m_DefaultValueBuffer.Write((byte*)&mat3Value, sizeof(Matrix3), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			case ParserPropertyType::Matrix4:
			{
				Matrix4 mat4Value = Parse<Matrix4>(value);
				m_DefaultValueBuffer.Write((byte*)&mat4Value, sizeof(Matrix4), m_Declaration.GetUniformDeclarations()[i]->GetOffset());
				break;
			}
			default:
				break;
			}
		}
	}

	#pragma endregion
}