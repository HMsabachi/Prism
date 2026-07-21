#pragma once

#include "Prism/Renderer/Shader.h"
#include <unordered_map>

namespace Prism {

    class PRISM_API OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource);
        OpenGLShader(const std::string& computeSource);
        virtual ~OpenGLShader();

        virtual void Reload() override {}
        virtual void Bind() override;

        virtual void SetInt(const std::string& name, int value) override;
        virtual void SetFloat(const std::string& name, float value) override;
        virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

        RendererID GetRendererID() const override { return m_RendererID; }
        const std::string& GetName() const override { return m_Name; }

        void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) override;

    private:
        void CompileAndUploadShader();

        void UploadUniformInt(const std::string& name, int32_t value);
        void UploadUniformFloat(const std::string& name, float value);
        void UploadUniformMat4(const std::string& name, const glm::mat4& value);
        bool UniformLocationCache(const std::string& name);

    private:
        RendererID m_RendererID = 0;
        std::string m_Name;
        std::string m_VertexSource;
        std::string m_FragmentSource;
        std::string m_ComputeSource;
        bool m_IsCompute = false;
        std::unordered_map<std::string, int> m_UniformLocationCache;
    };

}
