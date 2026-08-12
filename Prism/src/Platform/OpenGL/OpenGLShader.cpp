#include "prpch.h"
#include "OpenGLShader.h"

#include <glad/glad.h>
#include "Prism/Renderer/Renderer.h"

namespace Prism
{
    OpenGLShader::OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource)
        : m_VertexSource(vertexSource), m_FragmentSource(fragmentSource)
    {
        Ref<OpenGLShader> instance = this;
        Renderer::Submit([instance]() mutable { instance->CompileAndUploadShader(); });
    }

    OpenGLShader::OpenGLShader(const char* computeSource)
        : m_ComputeSource(computeSource), m_IsCompute(true)
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


    void OpenGLShader::RT_Bind() const
    {
        glUseProgram(m_RendererID);
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

}

