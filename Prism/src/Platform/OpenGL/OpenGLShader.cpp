#include "prpch.h"
#include "OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "Prism/Renderer/Renderer.h"

namespace Prism
{
    OpenGLShader::OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource)
        : m_Name(""), m_VertexSource(vertexSource), m_FragmentSource(fragmentSource)
    {
        Ref<OpenGLShader> instance = this;
        Renderer::Submit([instance]() mutable { instance->CompileAndUploadShader(); });
    }

    OpenGLShader::OpenGLShader(const std::string& computeSource)
        : m_Name(""), m_ComputeSource(computeSource), m_IsCompute(true)
    {
        Ref<OpenGLShader> instance = this;
        Renderer::Submit([instance]() mutable { instance->CompileAndUploadShader(); });
    }

    OpenGLShader::~OpenGLShader()
    {
        uint32_t id = m_RendererID;
        Renderer::Submit([id]() {
            if (id) glDeleteProgram(id);
        });
    }

    void OpenGLShader::Bind()
    {
        Ref<OpenGLShader> instance = this;
        Renderer::Submit([instance]() {
            glUseProgram(instance->m_RendererID);
        });
    }

    void OpenGLShader::CompileAndUploadShader()
    {
        std::vector<GLuint> shaderIDs;
        GLuint program = glCreateProgram();

        auto compileStage = [&](GLenum type, const std::string& source) -> bool {
            if (source.empty()) return true;
            GLuint id = glCreateShader(type);
            const GLchar* src = source.c_str();
            glShaderSource(id, 1, &src, 0);
            glCompileShader(id);

            GLint compiled = 0;
            glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
            if (compiled == GL_FALSE)
            {
                GLint maxLen = 0;
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLen);
                std::vector<GLchar> log(maxLen);
                glGetShaderInfoLog(id, maxLen, &maxLen, &log[0]);
                PR_CORE_ERROR("Shader compile failed:\n{0}", &log[0]);
                PR_CORE_WARN("Source: \n{0}", src);
                glDeleteShader(id);
                glDeleteProgram(program);
                for (auto sid : shaderIDs) glDeleteShader(sid);
                return false;
            }
            shaderIDs.push_back(id);
            glAttachShader(program, id);
            return true;
        };

        if (m_IsCompute)
        {
            if (!compileStage(GL_COMPUTE_SHADER, m_ComputeSource)) { m_RendererID = 0; return; }
        }
        else
        {
            if (!compileStage(GL_VERTEX_SHADER, m_VertexSource)) { m_RendererID = 0; return; }
            if (!compileStage(GL_FRAGMENT_SHADER, m_FragmentSource)) { m_RendererID = 0; return; }
        }
        glLinkProgram(program);
        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked == GL_FALSE)
        {
            GLint maxLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);
            std::vector<GLchar> log(maxLen + 1);
            glGetProgramInfoLog(program, maxLen, &maxLen, &log[0]);
            PR_CORE_ERROR("Shader link failed:\n{0}", &log[0]);
            PR_CORE_WARN("Source: \n{0}", m_VertexSource);
            PR_CORE_WARN("Source: \n{0}", m_FragmentSource);

            for (auto sid : shaderIDs) glDetachShader(program, sid);
            glDeleteProgram(program);
            m_RendererID = 0;
            return;
        }

        for (auto sid : shaderIDs) glDeleteShader(sid);
        m_RendererID = program;
    }

    void OpenGLShader::DispatchCompute(uint32_t x, uint32_t y, uint32_t z)
    {
        Renderer::Submit([=](){
            if (!m_IsCompute) { PR_CORE_WARN("Not a compute shader"); return; }
            glDispatchCompute(x, y, z);
        });
    }

    void OpenGLShader::SetInt(const std::string& name, int value)
    {
        Renderer::Submit([=]() { UploadUniformInt(name, value); });
    }

    void OpenGLShader::SetFloat(const std::string& name, float value)
    {
        Renderer::Submit([=]() { UploadUniformFloat(name, value); });
    }

    void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
    {
        Renderer::Submit([=]() { UploadUniformMat4(name, value); });
    }

    void OpenGLShader::UploadUniformInt(const std::string& name, int32_t value)
    {
        if (!UniformLocationCache(name)) return;
        glUniform1i(m_UniformLocationCache[name], value);
    }

    void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
    {
        if (!UniformLocationCache(name)) return;
        glUniform1f(m_UniformLocationCache[name], value);
    }

    void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& value)
    {
        if (!UniformLocationCache(name)) return;
        glUniformMatrix4fv(m_UniformLocationCache[name], 1, GL_FALSE, glm::value_ptr(value));
    }

    bool OpenGLShader::UniformLocationCache(const std::string& name)
    {
        if (!m_RendererID)
        {
            PR_CORE_WARN("Shader not compiled");
            return false;
        }
        auto it = m_UniformLocationCache.find(name);
        if (it == m_UniformLocationCache.end())
        {
            int location = glGetUniformLocation(m_RendererID, name.c_str());
            if (location == -1) return false;
            m_UniformLocationCache[name] = location;
        }
        return true;
    }

}

