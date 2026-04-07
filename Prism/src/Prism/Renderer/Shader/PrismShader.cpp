#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Utilities/Utilities.h"
#include "Platform/OpenGL/OpenGLShader.h"
#if 1

namespace Prism
{
	std::vector<PrismShader> PrismShader::s_AllPrismShader;

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
		m_PropertiesDeclaration = CreateScope<OpenGLShaderUniformBufferDeclaration>("Properties");

		PrismShaderParser parser;
		m_ParseResult = parser.Parse(source);
		m_Name = m_ParseResult.ShaderName;
		// 处理Property
		HandleProperty();
		std::string src = "#type vertex\n" + m_ParseResult.Passes[0].VertexShaderCode + "#type fragment\n" + m_ParseResult.Passes[0].FragmentShaderCode;
		m_Shader.reset(new OpenGLShader(m_Name, src));
		
		SetPropertiesToShader(m_PropertyBuffer);
	}
	
	void PrismShader::bind() const
	{
		m_Shader->Bind();
	}

	void PrismShader::SetPropertiesToShader(Buffer buffer)
	{
		m_Shader->ResolveAndSetUniforms(m_PropertiesDeclaration, buffer);
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

	Prism::Ref<Prism::Shader> PrismShader::GetOriginalShader() const
	{
		return m_Shader;
	}

	void PrismShader::HandleProperty()
	{
		size_t TextureIndex = 0;
		using namespace Prism::StrParse;
		using namespace Prism::ShaderData;
		for (auto& property : m_ParseResult.Properties)
		{
			ShaderData::PropertyElement element;
			element.m_Name = property.Name;
			element.m_DisplayName = property.DisplayName;
			PropertyValue value;
			switch (property.Type)
			{
			case PropertyType::Float:
				element.m_Type = PropertyElement::Type::Float;
				value = Parse<float>(property.DefaultValue);
				break;
			case PropertyType::Int:
				element.m_Type = PropertyElement::Type::Int;
				value = Parse<int32_t>(property.DefaultValue);
				break;
			case PropertyType::Color:
				element.m_Type = PropertyElement::Type::Color;
				value = Parse<glm::vec4>(property.DefaultValue);
				break;
			case PropertyType::Vector2:
				element.m_Type = PropertyElement::Type::Vector2;
				value = Parse<glm::vec2>(property.DefaultValue);
				break;
			case PropertyType::Vector3:
				element.m_Type = PropertyElement::Type::Vector3;
				value = Parse<glm::vec3>(property.DefaultValue);
				break;
			case PropertyType::Vector4:
				element.m_Type = PropertyElement::Type::Vector4;
				value = Parse<glm::vec4>(property.DefaultValue);
				break;
			case PropertyType::Texture2D:
			{
				element.m_Type = PropertyElement::Type::Texture2D;
				Texture2DType tex2d{ 0 };
				tex2d.id = TextureIndex++;
				value = tex2d;
				break;
			}
			case PropertyType::TextureCube:
			{
				element.m_Type = PropertyElement::Type::TextureCube;
				TextureCubeType texcube{ 0 };
				texcube.id = TextureIndex++;
				value = texcube;
				break;
			}
			case PropertyType::Range:
			{
				element.m_Type = PropertyElement::Type::Range;
				Range range{0.0f, 1.0f, 0.0f};
				range.min = property.Min;
				range.max = property.Max;
				range.value = Parse<float>(property.DefaultValue);
				value = range;
				break;
			}
			}
			element.m_DefaultValue = value;
			m_Properties.AddProperty(element);
		}
		HandlePropertyUniformBuffer();
		InitPropertyBuffer();
	}


	void PrismShader::HandlePropertyUniformBuffer()
	{
		for(auto& [name, element] : m_Properties)
		{
			m_PropertiesDeclaration->PushUniform(new OpenGLShaderUniformDeclaration(element.ToUniformType(element.GetType()), name));
		}
	}

	void PrismShader::InitPropertyBuffer()
	{
		m_PropertyBuffer.Allocate(m_PropertiesDeclaration->GetSize());
		for (auto& decl : m_PropertiesDeclaration->GetUniformDeclarations())
		{
			auto& element = m_Properties[decl->GetName()];
			auto [dataPtr, dataSize] = element.GetDefaultValueAsUniform();
			PR_CORE_INFO("{0} {1},{2},{3}", decl->GetName(),  (*(glm::vec3*)dataPtr).x, (*(glm::vec3*)dataPtr).y, (*(glm::vec3*)dataPtr).z);
			if(dataSize != decl->GetSize())
			{
				PR_CORE_WARN("Property {} 大小不匹配! 声明: {}, 实际: {}", element.GetName(), decl->GetSize(), dataSize);
				continue;
			}
			m_PropertyBuffer.Write(dataPtr, decl->GetSize(), decl->GetOffset());
		}
	}

	std::string PrismShader::ToString() const
	{
		const PrismShader& shader = *this;
		std::string result = fmt::format("PrismShader(Name: \"{}\", Path: \"{}\")\n",
			shader.GetName(), shader.GetFilePath());

		result += "Properties:\n";
		const auto& props = shader.GetProperties();

		if (props.empty()) {
			result += "  <None>";
			return result;
		}
		for (const auto& [name, element] : props)
		{
			std::string valStr = std::visit(Prism::Internal::ValueToString{}, element.GetValueVariant());

			result += fmt::format("  - [{}] {}: {} (Value: {})\n",
				(int)element.GetType(), 
				element.GetDisplayName(),
				element.GetName(),
				valStr
			);
		}
		return result;
	}

}
#endif