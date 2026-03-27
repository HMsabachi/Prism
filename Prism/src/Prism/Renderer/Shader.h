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
		static Shader* Create(const std::string& VertexShader, const std::string& FragmentShader);
	};
}

