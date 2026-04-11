#pragma once
#include <glm/glm.hpp>

namespace Prism
{
	namespace Type
	{
		typedef std::pair<byte*, size_t> BufferData; 

		typedef glm::vec4 Color; 
		typedef float Float;
		typedef int Int;
		typedef glm::vec2 Vector2;
		typedef glm::vec3 Vector3;
		typedef glm::vec4 Vector4;
		typedef glm::mat3 Matrix3;
		typedef glm::mat4 Matrix4;
		struct PRISM_API Range
		{
			float value;
			float min;
			float max;
			operator float() const;
			Range() = default;
			Range(float min, float max, float value);
			void SetValue(float v);
			void SetMin(float v);
			void SetMax(float v);
			std::string ToString() const;
		};

		std::string PRISM_API ToString(const Color& value);
		std::string PRISM_API ToString(const Vector2& value);
		std::string PRISM_API ToString(const Vector3& value);
		std::string PRISM_API ToString(const Vector4& value);
		std::string PRISM_API ToString(const Range& value);

		template <typename T>
		inline std::string format_as(const T& value)
		{
			if constexpr (std::is_same_v<T, Color>)
				return ToString(value);
			else if constexpr (std::is_same_v<T, Vector2>)
				return ToString(value);
			else if constexpr (std::is_same_v<T, Vector3>)
				return ToString(value);
			else if constexpr (std::is_same_v<T, Vector4>)
				return ToString(value);
			else if constexpr (std::is_same_v<T, Range>)
				return ToString(value);
			else
				return std::to_string(value);
		}
	}
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
		template <>
		inline glm::mat3 Parse<glm::mat3>(const std::string& input) {
			std::stringstream ss(Sanitize(input));
			float values;
			ss >> values;
			glm::mat3 m(values);
			return m;
		}
		template <>
		inline glm::mat4 Parse<glm::mat4>(const std::string& input) {
			std::stringstream ss(Sanitize(input));
			float values;
			ss >> values;
			glm::mat4 m(values);
			return m;
		}
	}
	namespace EnumParse
	{
		template <typename T>
		inline int32_t GetEnumValue(T value)
		{
			return static_cast<int32_t>(value);
		}
	}
}