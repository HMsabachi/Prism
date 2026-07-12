#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Renderer/RendererAPI.h"

#include <string>
#include <glm/glm.hpp>

namespace PrismShaderCompiler { struct PipelineState; }

namespace Prism
{
    class PRISM_API Shader : public RefCounted
    {
    public:
        virtual ~Shader() = default;
        virtual void Reload() = 0;

        virtual void Bind() = 0;

        virtual RendererID GetRendererID() const = 0;

        virtual const std::string& GetName() const = 0;

        virtual void ApplyRenderState(const PrismShaderCompiler::PipelineState& state) = 0;

        virtual void SetInt(const std::string& name, int value) = 0;
        virtual void SetFloat(const std::string& name, float value) = 0;
        virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

        virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) = 0;

        static Shader* Create(const std::string& vertexSource, const std::string& fragmentSource);
        static Shader* Create(const std::string& computeSource);

        static std::vector<Shader*> s_AllShaders;
    };

}
