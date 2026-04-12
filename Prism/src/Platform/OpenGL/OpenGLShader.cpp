#include "prpch.h"
#include "OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>


namespace Prism 
{
#define UNIFORM_LOGGING 1
#if UNIFORM_LOGGING 
	#define PR_LOG_UNIFORM(...) PR_CORE_WARN(__VA_ARGS__)
#else
	#define PR_LOG_UNIFORM(...)
#endif

	OpenGLShader::OpenGLShader(const std::string& filepath)
		:m_AssetsPath(filepath)
	{
		size_t found = filepath.find_last_of("/\\");
		m_Name = found != std::string::npos ? filepath.substr(found + 1) : filepath;
		Reload();
		
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& source)
		:m_Name(name), m_ShaderSource(source), m_RendererID(0), m_AssetsPath("")
	{
		PR_RENDER_S({
			self->CompileAndUploadShader();
		});
	}
	OpenGLShader::~OpenGLShader()
	{
		PR_RENDER_S({
			if (self->m_RendererID)
				glDeleteProgram(self->m_RendererID);
		});
	}

	void OpenGLShader::Reload()
	{
		if (m_AssetsPath == "")
		{
			PR_CORE_WARN("The shader from PrismShader!");
			return;
		}	
		ReadShaderFromFile(m_AssetsPath);
		PR_RENDER_S({
			if (self->m_RendererID)
				glDeleteProgram(self->m_RendererID);
		// TODO :这里加入Prismshader逻辑
			self->CompileAndUploadShader();
			});
	}

	void OpenGLShader::Bind()
	{
		PR_RENDER_S({
			glUseProgram(self->m_RendererID);
			});
	}

	void OpenGLShader::ReadShaderFromFile(const std::string& filepath)
	{
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			m_ShaderSource.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&m_ShaderSource[0], m_ShaderSource.size());
			in.close();
		}
		else
		{
			PR_CORE_WARN("Could not read shader file {0}", filepath);
		}
	}

	GLenum OpenGLShader::ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;

		return GL_NONE;
	}

	void OpenGLShader::CompileAndUploadShader()
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = m_ShaderSource.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = m_ShaderSource.find_first_of("\r\n", pos);
			PR_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = m_ShaderSource.substr(begin, eol - begin);
			PR_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel", "Invalid shader type specified");

			size_t nextLinePos = m_ShaderSource.find_first_not_of("\r\n", eol);
			pos = m_ShaderSource.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = m_ShaderSource.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? m_ShaderSource.size() - 1 : nextLinePos));
		}

		std::vector<GLuint> shaderRendererIDs;

		GLuint program = glCreateProgram();
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			std::string& source = kv.second;

			GLuint shaderRendererID = glCreateShader(type);
			const GLchar* sourceCstr = (const GLchar*)source.c_str();
			glShaderSource(shaderRendererID, 1, &sourceCstr, 0);

			glCompileShader(shaderRendererID);

			GLint isCompiled = 0;
			glGetShaderiv(shaderRendererID, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shaderRendererID, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shaderRendererID, maxLength, &maxLength, &infoLog[0]);

				PR_CORE_ERROR("Shader compilation failed:\n{0}", &infoLog[0]);
				PR_CORE_WARN("Shader Source:\n{0}", source);

				// We don't need the shader anymore.
				glDeleteShader(shaderRendererID);

				PR_CORE_ASSERT(false, "Failed");
			}

			shaderRendererIDs.push_back(shaderRendererID);
			glAttachShader(program, shaderRendererID);
		}

		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			PR_CORE_ERROR("Shader compilation failed:\n{0}", &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak Shaders either.
			for (auto id : shaderRendererIDs)
				glDeleteShader(id);
		}

		// Always detach Shaders after a successful link.
		for (auto id : shaderRendererIDs)
			glDeleteShader(id);

		//LoadTexture();

		m_RendererID = program;
	}

	void OpenGLShader::UploadUniformBuffer(const UniformBufferBase& uniformBuffer)
	{
		PR_RENDER_S({
			glUseProgram(self->m_RendererID);
		});
		for (unsigned int i = 0; i < uniformBuffer.GetUniformCount(); i++)
		{
			const UniformDecl& decl = uniformBuffer.GetUniforms()[i];
			switch (decl.Type)
			{
			case UniformType::Int32:
			{
				const std::string& name = decl.Name;
				int value = *(int*)(uniformBuffer.GetBuffer() + decl.Offset);
				PR_RENDER_S2(name, value, {
					self->UploadUniformInt(name, value);
				});
				break;
			}
			case UniformType::Float:
			{
				const std::string& name = decl.Name;
				float value = *(float*)(uniformBuffer.GetBuffer() + decl.Offset);
				PR_RENDER_S2(name, value, {
					self->UploadUniformFloat(name, value);
					});
				break;
			}
			case UniformType::Float3:
			{
				const std::string& name = decl.Name;
				glm::vec3& values = *(glm::vec3*)(uniformBuffer.GetBuffer() + decl.Offset);
				PR_RENDER_S2(name, values, {
					self->UploadUniformFloat3(name, values);
					});
				break;
			}
			case UniformType::Float4:
			{
				const std::string& name = decl.Name;
				glm::vec4& values = *(glm::vec4*)(uniformBuffer.GetBuffer() + decl.Offset);
				PR_RENDER_S2(name, values, {
					self->UploadUniformFloat4(name, values);
					});
				break;
			}
			case UniformType::Matrix4x4:
			{
				const std::string& name = decl.Name;
				glm::mat4& values = *(glm::mat4*)(uniformBuffer.GetBuffer() + decl.Offset);
				PR_RENDER_S2(name, values, {
					self->UploadUniformMat4(name, values);
					});
				break;
			}
			}
		}
	}
	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		PR_RENDER_S2(name, value, {
			self->UploadUniformFloat(name, value);
			});
	}
	void OpenGLShader::SetVec3(const std::string& name, const glm::vec3& value)
	{
		PR_RENDER_S2(name, value, {
			self->UploadUniformFloat3(name, value);
			});
	}
	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		PR_RENDER_S2(name, value, {
			self->UploadUniformMat4(name, value);
			});
	}

	void OpenGLShader::SetMat4FromRenderThread(const std::string& name, const glm::mat4& value)
	{
		glUseProgram(m_RendererID);
		UploadUniformMat4(name, value);
	}



	void OpenGLShader::SetProperty(const PropertyBufferDeclaration& decl, const Buffer& buffer)
	{
		PR_RENDER_S2(decl, buffer, {
			self->SetPropertyImpt(decl, buffer);
		});
	}

	void OpenGLShader::SetPropertyImpt(const PropertyBufferDeclaration& decl, const Buffer& buffer)
	{
		glUseProgram(m_RendererID);
		using namespace Prism::PropertyType;
		for (const auto& property : decl)
		{
			uint32_t offset = property->GetOffset();
			switch (property->GetType())
			{
			case PropertyDeclaration::Type::Color:
				UploadUniformFloat4(property->GetName(), *(Color*)&buffer.Data[offset]);
				break;
			case PropertyDeclaration::Type::Float:
				UploadUniformFloat(property->GetName(), *(Float*)&buffer.Data[offset]);
				break;
			case PropertyDeclaration::Type::Int:
				UploadUniformInt(property->GetName(), *(Int*)&buffer.Data[offset]);
				break;
			case PropertyDeclaration::Type::Vector2:
				UploadUniformFloat2(property->GetName(), *(Vector2*)&buffer.Data[offset]);
				break;
			case PropertyDeclaration::Type::Vector3:
			{
				UploadUniformFloat3(property->GetName(), *(Vector3*)&buffer.Data[offset]);
				break;
			}
			case PropertyDeclaration::Type::Vector4:
				UploadUniformFloat4(property->GetName(), *(Vector4*)&buffer.Data[offset]);
				break;
			case PropertyDeclaration::Type::Range:
				UploadUniformFloat(property->GetName(), *(Range*)&buffer.Data[offset]);
				break;
			case PropertyDeclaration::Type::Texture2D:
			{
				auto& tex = *(PropertyType::Texture2D*)&buffer.Data[offset];
				UploadUniformInt(property->GetName(), tex.slot);
				break;
			}
			case PropertyDeclaration::Type::TextureCube:
				UploadUniformInt(property->GetName(), *(PropertyType::TextureCube*)&buffer.Data[offset]);
				break;
			case PropertyDeclaration::Type::Matrix3:
				UploadUniformMat3(property->GetName(), *(glm::mat3*)&buffer.Data[offset]);
				break;
			case PropertyDeclaration::Type::Matrix4:
				UploadUniformMat4(property->GetName(), *(glm::mat4*)&buffer.Data[offset]);
				break;
			default:
				PR_LOG_UNIFORM("不支持的属性类型{0} 在Shader {1}中", property->GetName(), m_Name);
				break;
			}
		}
	}


#pragma region 设置uniform变量
	void OpenGLShader::UploadUniformInt(uint32_t location, int32_t value)
	{
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(uint32_t location, int32_t* values, int32_t count)
	{
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(uint32_t location, float value)
	{
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(uint32_t location, const glm::vec2& value)
	{
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(uint32_t location, const glm::vec3& value)
	{
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(uint32_t location, const glm::vec4& value)
	{
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(uint32_t location, const glm::mat3& value)
	{
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::UploadUniformMat4(uint32_t location, const glm::mat4& value)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::UploadUniformMat4Array(uint32_t location, const glm::mat4& values, uint32_t count)
	{
		glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(values));
	}

	void OpenGLShader::UploadUniformInt(std::string name, int32_t value)
	{
		if (!UniformLocationCache(name))
			return;
		UploadUniformInt(m_UniformLocationCache[name], value);
	}
	void OpenGLShader::UploadUniformIntArray(std::string name, int32_t* values, int32_t count)
	{
		if (!UniformLocationCache(name))
			return;
		UploadUniformIntArray(m_UniformLocationCache[name], values, count);
	}
	void OpenGLShader::UploadUniformFloat(std::string name, float value)
	{
		if (!UniformLocationCache(name))
			return;
		UploadUniformFloat(m_UniformLocationCache[name], value);
	}
	void OpenGLShader::UploadUniformFloat2(std::string name, const glm::vec2& value)
	{
		if (!UniformLocationCache(name))
			return;
		UploadUniformFloat2(m_UniformLocationCache[name], value);
	}
	void OpenGLShader::UploadUniformFloat3(std::string name, const glm::vec3& value)
	{
		if (!UniformLocationCache(name))
			return;
		UploadUniformFloat3(m_UniformLocationCache[name], value);
	}
	void OpenGLShader::UploadUniformFloat4(std::string name, const glm::vec4& value)
	{
		if (!UniformLocationCache(name))
			return;
		UploadUniformFloat4(m_UniformLocationCache[name], value);
	}
	void OpenGLShader::UploadUniformMat3(std::string name, const glm::mat3& values)
	{
		if (!UniformLocationCache(name))
			return;
		UploadUniformMat3(m_UniformLocationCache[name], values);
	}
	void OpenGLShader::UploadUniformMat4(std::string name, const glm::mat4& values)
	{
		if (!UniformLocationCache(name))
		{
			PR_LOG_UNIFORM("Uniform {0} 在Shader {1}中不存在", name, m_Name);
			return;
		}
		UploadUniformMat4(m_UniformLocationCache[name], values);
	}
	void OpenGLShader::UploadUniformMat4Array(std::string name, const glm::mat4& values, uint32_t count)
	{
		if (!UniformLocationCache(name))
			return;
		UploadUniformMat4Array(m_UniformLocationCache[name], values, count);
	}

	bool OpenGLShader::UniformLocationCache(std::string& name)
	{
		auto it = m_UniformLocationCache.find(name);
		if (it == m_UniformLocationCache.end())
		{
			int location = glGetUniformLocation(m_RendererID, name.c_str());
			if (location == -1)
			{
				PR_LOG_UNIFORM("{0}中没有找到{1}", m_Name, name);
				return false;
			}
			m_UniformLocationCache[name] = location;
		}
		return true;
	}


#pragma endregion



#pragma region Private Methods
	void OpenGLShader::LoadTexture()
	{
		// Bind default texture unit
		UploadUniformInt("u_Texture", 0);

		// PBR shader Textures
		UploadUniformInt("u_AlbedoTexture", 1);
		UploadUniformInt("u_NormalTexture", 2);
		UploadUniformInt("u_MetalnessTexture", 3);
		UploadUniformInt("u_RoughnessTexture", 4);

		UploadUniformInt("u_EnvRadianceTex", 10);
		UploadUniformInt("u_EnvIrradianceTex", 11);

		UploadUniformInt("u_BRDFLUTTexture", 15);
	}

#pragma endregion


}


