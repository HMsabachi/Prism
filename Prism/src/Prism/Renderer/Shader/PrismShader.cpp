#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Utilities/Utilities.h"
#if 1

namespace Prism
{
	// //////////////////////////////////////////////////
	// --- PrismShader  ---
	// //////////////////////////////////////////////////
	std::vector<Ref<PrismShader>> PrismShader::s_AllShaders;

	Ref<PrismShader> PrismShader::Create(const std::string& path, const bool isFile)
	{
		Ref<PrismShader> shader = CreateRef<PrismShader>(path);
		s_AllShaders.push_back(shader);
		return shader;
	}

	PrismShader::PrismShader(const std::string& path)
	{
		m_FilePath = path;
		Reload();
	}
	PrismShader::~PrismShader()
	{

	}
	
	void PrismShader::Reload()
	{
		auto source = ReadFile(m_FilePath);
		PrismShaderParser parser;
		m_ParseResult = parser.Parse(source);
		m_Name = m_ParseResult.ShaderName;
		m_Shader.reset(Shader::Create(m_Name, m_ParseResult.Passes[0].VertexShaderCode, m_ParseResult.Passes[0].FragmentShaderCode));
		m_ShaderProperty.Init(m_ParseResult.Properties);
		SetProperty(m_ShaderProperty.GetDefaultValueBuffer());
		for (const auto& callback : m_ReloadedCallbacks)
			callback();
	}

	void PrismShader::Bind() const
	{
		m_Shader->Bind();
	}

	void PrismShader::SetProperty(const Buffer& buffer)
	{
		m_Shader->SetProperty(m_ShaderProperty.GetDeclaration(), buffer);
	}

	void PrismShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
	{
		m_ReloadedCallbacks.push_back(callback);
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

	Prism::Ref<Prism::Shader> PrismShader::GetOriginalShader() const
	{
		return m_Shader;
	}

	std::string PrismShader::ToString() const
	{
		const PrismShader& shader = *this;
		std::string result = fmt::format("PrismShader(Name: \"{}\", Path: \"{}\")\n",
			shader.GetName(), shader.GetFilePath());

		result += "Properties:\n";
		const auto& buffer = shader.GetProperty().GetDefaultValueBuffer();
		for (const auto& decl : shader.GetProperty().GetDeclaration())
		{
			auto type = decl->GetType();
			auto offset = decl->GetOffset();
			switch (type)
			{
			case Prism::PropertyDeclaration::Type::None:
				break;
			case Prism::PropertyDeclaration::Type::Color:
				result += fmt::format("  - {} (Color): {}\n", decl->GetName(), Type::ToString(*(PropertyType::Color*)&buffer.Data[offset]));
				break;
			case Prism::PropertyDeclaration::Type::Float:
				result += fmt::format("  - {} (Float): {}\n", decl->GetName(), *(PropertyType::Float*)&buffer.Data[offset]);
				break;
			case Prism::PropertyDeclaration::Type::Int:
				result += fmt::format("  - {} (Int): {}\n", decl->GetName(), *(PropertyType::Int*)&buffer.Data[offset]);
				break;
			case Prism::PropertyDeclaration::Type::Vector2:
				result += fmt::format("  - {} (Vector2): {}\n", decl->GetName(), Type::ToString(*(PropertyType::Vector2*)&buffer.Data[offset]));
				break;
			case Prism::PropertyDeclaration::Type::Vector3:
				result += fmt::format("  - {} (Vector3): {}\n", decl->GetName(), Type::ToString(*(PropertyType::Vector3*)&buffer.Data[offset]));
				break;
			case Prism::PropertyDeclaration::Type::Vector4:
				result += fmt::format("  - {} (Vector4): {}\n", decl->GetName(), Type::ToString(*(PropertyType::Vector4*)&buffer.Data[offset]));
				break;
			case Prism::PropertyDeclaration::Type::Range:
				result += fmt::format("  - {} (Range): {}\n", decl->GetName(), Type::ToString(*(PropertyType::Range*)&buffer.Data[offset]));
				break;
			case Prism::PropertyDeclaration::Type::Texture2D:
				result += fmt::format("  - {} (Texture2D): Slot {}\n", decl->GetName(), ((PropertyType::Texture2D*)&buffer.Data[offset])->slot);
				break;
			case Prism::PropertyDeclaration::Type::TextureCube:
				result += fmt::format("  - {} (TextureCube): Slot {}\n", decl->GetName(), ((PropertyType::TextureCube*)&buffer.Data[offset])->slot);
				break;
			default:
				break;
			}
		}
		return result;
	}

	void PrismShader::SetMat4FromRenderThread(const std::string& name, const glm::mat4& value)
	{
		m_Shader->SetMat4FromRenderThread(name, value);
	}

	// //////////////////////////////////////////////////
	// --- ShaderLibrary  ---
	// //////////////////////////////////////////////////

	ShaderLibrary::ShaderLibrary()
	{
	}

	ShaderLibrary::~ShaderLibrary()
	{
	}

	void ShaderLibrary::Add(const Ref<PrismShader>& shader)
	{
		auto& name = shader->GetName();
		PR_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Load(const std::string& path)
	{
		auto shader = Ref<PrismShader>(PrismShader::Create(path));
		auto& name = shader->GetName();
		PR_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Load(const std::string& name, const std::string& path)
	{
		PR_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());
		m_Shaders[name] = Ref<PrismShader>(PrismShader::Create(path));
	}

	Ref<PrismShader>& ShaderLibrary::Get(const std::string& name)
	{
		PR_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end());
		return m_Shaders[name];
	}

}
#endif