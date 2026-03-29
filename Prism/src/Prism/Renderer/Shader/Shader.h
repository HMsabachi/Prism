#pragma once

#include "Prism/Core/Core.h"
#include <string>
#include <glm/glm.hpp>

namespace Prism
{
	class PRISM_API Shader
	{
	public:
		
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	public:
		virtual void SetInt(const std::string& Name, int Value) const = 0;
		virtual void SetIntArray(const std::string& Name, int* Values, uint32_t Count) const = 0;
		virtual void SetFloat(const std::string& Name, float Value) const = 0;
		virtual void SetFloat2(const std::string& Name, const glm::vec2& Value) const = 0;
		virtual void SetFloat3(const std::string& Name, const glm::vec3& Value) const = 0;
		virtual void SetFloat4(const std::string& Name, const glm::vec4& Value) const = 0;
		virtual void SetMat3(const std::string& Name, const glm::mat3& Value) const = 0;
		virtual void SetMat4(const std::string& Name, const glm::mat4& Value) const = 0;
		

	public:
		static Ref<Shader> Create(const std::string& VertexShader, const std::string& FragmentShader);
	};
}

