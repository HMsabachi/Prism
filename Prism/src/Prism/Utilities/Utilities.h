#pragma once
#include <glm/glm.hpp>

namespace Prism
{
	namespace StrParse
	{
		std::string Sanitize(std::string str);


		template <typename T>
		inline T Parse(const std::string& input) {
			std::stringstream ss(Sanitize(input));
			T result{};
			ss >> result;
			return result;
		}

		template <>
		inline glm::vec2 Parse<glm::vec2>(const std::string& input) {
			std::stringstream ss(Sanitize(input));
			glm::vec2 v(0.0f);
			ss >> v.x >> v.y;
			return v;
		}
		template <>
		inline glm::vec3 Parse<glm::vec3>(const std::string& input) {
			std::stringstream ss(Sanitize(input));
			glm::vec3 v(0.0f);
			ss >> v.x >> v.y >> v.z;
			return v;
		}
		template <>
		inline glm::vec4 Parse<glm::vec4>(const std::string& input) {
			std::stringstream ss(Sanitize(input));
			glm::vec4 v(0.0f);
			ss >> v.x >> v.y >> v.z >> v.w;
			return v;
		}
	}
}