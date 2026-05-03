#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Renderer/Shader/Parser/ShaderParser.h"
#include "Prism/Utilities/Utilities.h"

namespace Prism
{
	std::vector<Ref<PrismShader>> PrismShader::s_AllShaders;

	Ref<PrismShader> PrismShader::Create(const std::string& path)
	{
		Ref<PrismShader> shader = Ref<PrismShader>::Create(path);
		s_AllShaders.push_back(shader);
		return shader;
	}

	Ref<PrismShader> PrismShader::CreateFromString(const std::string& source)
	{
		Ref<PrismShader> shader = Ref<PrismShader>::Create();
		shader->Load(source);
		s_AllShaders.push_back(shader);
		return shader;
	}

	PrismShader::PrismShader(const std::string& path)
	{
		m_FilePath = std::filesystem::absolute(path).string();
		Reload();
	}

	PrismShader::PrismShader()
	{
	}

	PrismShader::~PrismShader()
	{
	}

	void PrismShader::Reload()
	{
		auto source = File::ReadFile(m_FilePath);
		Load(source);
	}

	void PrismShader::Load(const std::string& source)
	{
		ShaderParser parser;
		ParseResult result = parser.Parse(source);
		if (!result.Success)
		{
			PR_CORE_ERROR("Failed to parse shader file '{0}'", m_FilePath);
			return;
		}

		m_Name = result.ShaderName;

		m_Shader.Reset(Shader::Create(m_Name, result.Passes[0].VertexShaderCode, result.Passes[0].FragmentShaderCode));
		m_ShaderCommand = ParseShaderCommand(result.RenderCommand);

		m_Declaration = PropertyBufferDeclaration();
		for (const auto& prop : result.Properties)
		{
			auto& decl = m_Declaration.AddProperty(
				PropertyDeclaration(prop.Type, prop.Name, prop.DisplayName)
			);
			if (prop.Type == PropertyDeclarationType::Enum)
				decl.SetEnumOptions(prop.EnumOptions);
		}
		PackDefaultValues(result.Properties);

		SetProperty(m_DefaultValueBuffer);

		for (const auto& callback : m_ReloadedCallbacks)
			callback();
	}

	void PrismShader::PackDefaultValues(const std::vector<PropertyDescriptor>& properties)
	{
		using namespace Prism::StrParse;
		using namespace Prism::PropertyType;

		m_DefaultValueBuffer.Allocate(m_Declaration.GetSize());

		auto declIt = m_Declaration.begin();
		for (size_t i = 0; i < properties.size(); ++i, ++declIt)
		{
			const auto& prop = properties[i];
			uint32_t offset = declIt->GetOffset();

			switch (prop.Type)
			{
			case PropertyDeclarationType::Bool:
			{
				Bool val = (prop.DefaultValue == "true" || prop.DefaultValue == "1");
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Bool), offset);
				break;
			}
			case PropertyDeclarationType::Color:
			{
				Color val = Parse<Color>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Color), offset);
				break;
			}
			case PropertyDeclarationType::Float:
			{
				Float val = Parse<Float>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Float), offset);
				break;
			}
			case PropertyDeclarationType::Int:
			{
				Int val = Parse<Int>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Int), offset);
				break;
			}
			case PropertyDeclarationType::Vector2:
			{
				Vector2 val = Parse<Vector2>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Vector2), offset);
				break;
			}
			case PropertyDeclarationType::Vector3:
			{
				Vector3 val = Parse<Vector3>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Vector3), offset);
				break;
			}
			case PropertyDeclarationType::Vector4:
			{
				Vector4 val = Parse<Vector4>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Vector4), offset);
				break;
			}
			case PropertyDeclarationType::Texture2D:
			case PropertyDeclarationType::Texture2DMS:
			{
				PropertyType::Texture2D tex;
				tex.slot = m_NextTexSlot++;
				m_DefaultValueBuffer.Write((byte*)&tex, sizeof(PropertyType::Texture2D), offset);
				break;
			}
			case PropertyDeclarationType::TextureCube:
			{
				PropertyType::TextureCube tex;
				tex.slot = m_NextTexSlot++;
				m_DefaultValueBuffer.Write((byte*)&tex, sizeof(PropertyType::TextureCube), offset);
				break;
			}
			case PropertyDeclarationType::Range:
			{
				Range val;
				val.min = prop.Min;
				val.max = prop.Max;
				val.value = Parse<float>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Range), offset);
				break;
			}
			case PropertyDeclarationType::Matrix3:
			{
				Matrix3 val = Parse<Matrix3>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Matrix3), offset);
				break;
			}
			case PropertyDeclarationType::Matrix4:
			{
				Matrix4 val = Parse<Matrix4>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Matrix4), offset);
				break;
			}
			case PropertyDeclarationType::Enum:
			{
				Int val = Parse<Int>(prop.DefaultValue);
				m_DefaultValueBuffer.Write((byte*)&val, sizeof(Int), offset);
				break;
			}
			default:
				break;
			}
		}
	}

	uint32_t PrismShader::GetTextureSlot(const std::string& name) const
	{
		const PropertyDeclaration* decl = m_Declaration.FindProperty(name);
		switch (decl->GetType())
		{
		case PropertyDeclarationType::Texture2D:
		case PropertyDeclarationType::Texture2DMS:
		case PropertyDeclarationType::TextureCube:
			break;
		default:
			PR_CORE_ERROR("Property {0} is not a texture type!", name);
			PR_CORE_ASSERT(false, "");
		}
		return decl->GetValue<uint32_t>(m_DefaultValueBuffer);
	}

	void PrismShader::Bind()
	{
		m_Shader->Bind();
		m_Shader->ApplyCommand(m_ShaderCommand);
	}

	void PrismShader::SetProperty(const Buffer& buffer)
	{
		m_Shader->SetProperty(m_Declaration, buffer);
	}

	void PrismShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
	{
		m_ReloadedCallbacks.push_back(callback);
	}

	Ref<Shader> PrismShader::GetOriginalShader() const
	{
		return m_Shader;
	}

#pragma region Native uniforms

	void PrismShader::SetMat4FromRenderThread(const std::string& name, const glm::mat4& value)
	{
		m_Shader->SetMat4FromRenderThread(name, value);
	}
	void PrismShader::SetInt(const std::string& name, int value)
	{
		m_Shader->SetInt(name, value);
	}
	void PrismShader::SetIntArray(const std::string& name, int* values, uint32_t size)
	{
		m_Shader->SetIntArray(name, values, size);
	}
	void PrismShader::SetFloat(const std::string& name, float value)
	{
		m_Shader->SetFloat(name, value);
	}
	void PrismShader::SetVec3(const std::string& name, const glm::vec3& value)
	{
		m_Shader->SetVec3(name, value);
	}
	void PrismShader::SetVec4(const std::string& name, const glm::vec4& value)
	{
		m_Shader->SetVec4(name, value);
	}
	void PrismShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		m_Shader->SetMat4(name, value);
	}

#pragma endregion

	// //////////////////////////////////////////////////
	// --- ShaderLibrary  ---
	// //////////////////////////////////////////////////

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

	void ShaderLibrary::LoadAll(const std::string& directory)
	{
		PR_CORE_INFO("正在扫描 '{0}' 中的 .Shader 文件...", directory);
		uint32_t success = 0, failed = 0;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			if (entry.path().extension() != ".Shader")
				continue;

			std::string path = entry.path().string();
			auto shader = Ref<PrismShader>(PrismShader::Create(path));

			if (shader->GetName().empty())
			{
				PR_CORE_ERROR("解析失败，跳过: {0}", path);
				failed++;
				continue;
			}

			auto& name = shader->GetName();
			if (m_Shaders.find(name) != m_Shaders.end())
			{
				PR_CORE_WARN("重复的Shader名称 '{0}'，来自: {1}", name, path);
				continue;
			}
			m_Shaders[name] = shader;
			success++;
		}
		PR_CORE_INFO("Shader加载完成: {0} 个成功, {1} 个失败", success, failed);
	}

	const Ref<PrismShader>& ShaderLibrary::Get(const std::string& name) const
	{
		PR_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end());
		return m_Shaders.at(name);
	}
}
