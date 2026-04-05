#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Utilities/Utilities.h"
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
		PrismShaderParser parser;
		m_ParseResult = parser.Parse(source);

		// 处理Property
		HandleProperty();

		m_Shader.reset(Shader::Create(m_ParseResult.ShaderName, m_ParseResult.Passes[0].VertexShaderCode, m_ParseResult.Passes[0].FragmentShaderCode));

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

	void PrismShader::HandleProperty()
	{
		size_t TextureIndex = 0;
		using namespace Prism::StrParse;
		for (auto& property : m_ParseResult.Properties)
		{
			ShaderData::PropertyElement element;
			element.m_Name = property.Name;
			element.m_DisplayName = property.DisplayName;
			PropertyValue value;
			switch (property.Type)
			{
			case PropertyType::Float:
				element.m_Type = UniformType::Float;
				value = Parse<float>(property.DefaultValue);
				break;
			case PropertyType::Int:
				element.m_Type = UniformType::Int32;
				value = Parse<int32_t>(property.DefaultValue);
				break;
			case PropertyType::Color:
				element.m_Type = UniformType::Float4;
				value = Parse<glm::vec4>(property.DefaultValue);
				break;
			case PropertyType::Vector2:
				element.m_Type = UniformType::Float2;
				value = Parse<glm::vec2>(property.DefaultValue);
				break;
			case PropertyType::Vector3:
				element.m_Type = UniformType::Float3;
				value = Parse<glm::vec3>(property.DefaultValue);
				break;
			case PropertyType::Vector4:
				element.m_Type = UniformType::Float4;
				value = Parse<glm::vec4>(property.DefaultValue);
				break;
			case PropertyType::Texture2D:
				element.m_Type = UniformType::Texture2D;
				Texture2DType tex2d;
				tex2d.id = TextureIndex++;
				value = tex2d;
				break;
			case PropertyType::TextureCube:
				element.m_Type = UniformType::TextureCube;
				TextureCubeType texcube;
				texcube.id = TextureIndex++;
				value = texcube;
				break;
			case PropertyType::Range: // TODO: 这里暂时将Range当作Float处理
				element.m_Type = UniformType::Float;
				value = Parse<float>(property.DefaultValue);
				break;
			}
			element.m_DefaultValue = value;
			m_Properties.AddProperty(element);
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