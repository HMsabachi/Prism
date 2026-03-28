#include "prpch.h"
#include "OpenGLGlobalUniform.h"

#include <glad/glad.h>

namespace Prism
{
    OpenGLGlobalUniform::OpenGLGlobalUniform()
    {
        CreateGlobalUniformBuffer();
    }
    OpenGLGlobalUniform::~OpenGLGlobalUniform()
    {
        glDeleteBuffers(1, &m_GlobalUBO);
    }
    void OpenGLGlobalUniform::CreateGlobalUniformBuffer()
    {
        glGenBuffers(1, &m_GlobalUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(PrismGlobalsUBO), nullptr, GL_DYNAMIC_DRAW);  // 动态更新
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_GlobalUBO);   // binding = 0
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void OpenGLGlobalUniform::UpdateGlobalUniformBuffer(const PrismGlobalsUBO& globalUniforms) const
    {
        // 填充数据（在 BeginScene 中先更新 m_GlobalsData）
        glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PrismGlobalsUBO), &globalUniforms);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}