#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Renderer/RendererAPI.h"

#include <string>
#include <glm/glm.hpp>

namespace Prism
{
	struct ShaderCommand;
	class PropertyBufferDeclaration;
}

namespace Prism
{
	class PRISM_API Shader
	{
	public:
		virtual ~Shader() = default;
		virtual void Reload() = 0;

		virtual void Bind() = 0;

		virtual RendererID GetRendererID() const = 0;

		virtual const std::string& GetName() const = 0;

		virtual void SetProperty(const PropertyBufferDeclaration& decl, const Buffer& buffer) = 0;
		virtual void ApplyCommand(const ShaderCommand& command) = 0;

		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetVec2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetVec3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetVec4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetMat4FromRenderThread(const std::string& name, const glm::mat4& value, bool bind = true) = 0;

		virtual void SetIntArray(const std::string& name, int* values, uint32_t size) = 0;

		virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) = 0;
	public:
		static Shader* Create(const std::string& filepath);
		static Shader* Create(const std::string& name, const std::string& vertexShader, const std::string& fragmentShader);
		static Shader* Create(const std::string& name, const std::string& computeShader);

		static std::vector<Shader*> s_AllShaders;
	};

}