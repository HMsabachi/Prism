#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Renderer/Renderer.h"
#include "Shader/ShaderProperty.h"

#include <string>
#include <glm/glm.hpp>

namespace Prism
{
	struct PRISM_API ShaderUniform
	{

	};

	struct PRISM_API ShaderUniformCollection
	{

	};

	enum class PRISM_API UniformType
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Matrix3x3, Matrix4x4,
		Int32, Uint32,
		Texture2D, TextureCube
	};

	struct PRISM_API UniformDecl
	{
		UniformType Type;
		std::ptrdiff_t Offset;
		std::string Name;
	};

	struct PRISM_API UniformBuffer
	{
		// TODO: This currently represents a byte buffer that has been
		// packed with uniforms. This was primarily created for OpenGL,
		// and needs to be revisted for other rendering APIs. Furthermore,
		// this currently does not assume any alignment. This also has
		// nothing to do with GL uniform buffers, this is simply a CPU-side
		// buffer abstraction.
		byte* Buffer;
		std::vector<UniformDecl> Uniforms;
	};

	struct UniformBufferBase
	{
		virtual const byte* GetBuffer() const = 0;
		virtual const UniformDecl* GetUniforms() const = 0;
		virtual unsigned int GetUniformCount() const = 0;
	};

	template<unsigned int N, unsigned int U>
	struct UniformBufferDeclaration : public UniformBufferBase
	{
		byte Buffer[N];
		UniformDecl Uniforms[U];
		std::ptrdiff_t Cursor = 0;
		int Index = 0;

		virtual const byte* GetBuffer() const override { return Buffer; }
		virtual const UniformDecl* GetUniforms() const override { return Uniforms; }
		virtual unsigned int GetUniformCount() const { return U; }

		template<typename T>
		void Push(const std::string& name, const T& data) {
			PR_CORE_WARN("Uniform buffer declaration is not implemented for type {0} named {1}", typeid(T).name(), name);
		}
		template<>
		void Push(const std::string& name, const int& data)
		{
			Uniforms[Index++] = { UniformType::Int32, Cursor, name };
			memcpy(Buffer + Cursor, &data, sizeof(int));
			Cursor += sizeof(int);
		}
		template<>
		void Push(const std::string& name, const float& data)
		{
			Uniforms[Index++] = { UniformType::Float, Cursor, name };
			memcpy(Buffer + Cursor, &data, sizeof(float));
			Cursor += sizeof(float);
		}
		template<>
		void Push(const std::string& name, const glm::vec3& data)
		{
			Uniforms[Index++] = { UniformType::Float3, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec3));
			Cursor += sizeof(glm::vec3);
		}

		template<>
		void Push(const std::string& name, const glm::vec4& data)
		{
			Uniforms[Index++] = { UniformType::Float4, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec4));
			Cursor += sizeof(glm::vec4);
		}
		template<>
		void Push(const std::string& name, const glm::mat4& data)
		{
			Uniforms[Index++] = { UniformType::Matrix4x4, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::mat4));
			Cursor += sizeof(glm::mat4);
		}

	};


	class PRISM_API Shader
	{
	public:
		virtual ~Shader() = default;
		virtual void Reload() = 0;

		virtual void Bind() = 0;
		virtual void UploadUniformBuffer(const UniformBufferBase& uniformBuffer) = 0;



		// Temporary while we don't have materials
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetVec3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual const std::string& GetName() const = 0;

		virtual void SetProperty(const PropertyBufferDeclaration& decl, const Buffer& buffer) = 0;

		// Represents a complete shader program stored in a single file.
		// Note: currently for simplicity this is simply a string filepath, however
		//       in the future this will be an asset object + metadata
		static Shader* Create(const std::string& filepath);
		static Shader* Create(const std::string& name, const std::string& vertexShader, const std::string& fragmentShader);

		static std::vector<Shader*> s_AllShaders;
	};

}