#pragma once
#include "Prism/Renderer/Shader/ShaderUniform.h"

namespace Prism
{
	typedef std::pair<byte*, size_t> UniformData;

	class OpenGLShaderUniformDeclaration : public ShaderUniformDeclaration
	{
	public:
		friend class OpenGLShaderUniformBufferDeclaration;
	public:
		enum class Type
		{
			NONE, FLOAT32, VEC2, VEC3, VEC4, MAT3, MAT4, INT32, STRUCT, TEXTURE2D, TEXTURECUBE
		};
	private:
		std::string m_Name;
		uint32_t m_Size;
		uint32_t m_Count;
		uint32_t m_Offset;
		Type m_Type;
	public:
		OpenGLShaderUniformDeclaration(Type type, const std::string& name, uint32_t count = 1);


		const std::string& GetName() const override { return m_Name; }
		uint32_t GetSize() const override { return m_Size; }
		uint32_t GetCount() const override { return m_Count; }
		uint32_t GetOffset() const override { return m_Offset; }

		inline Type GetType() const { return m_Type; }
		inline bool IsArray() const { return m_Count > 1; }
	protected:
		void SetOffset(uint32_t offset) override { m_Offset = offset; }

	#pragma region 类型计算方法
	private:
		static uint32_t SizeOfUniformType(Type type);
		static Type StringToType(const std::string& type);
		static std::string TypeToString(Type type);
	#pragma endregion
	};

	class OpenGLShaderUniformBufferDeclaration : public ShaderUniformBufferDeclaration
	{
	private:
		friend class Shader;
	private:
		std::string m_Name;
		ShaderUniformList m_Uniforms;
		uint32_t m_Register;
		uint32_t m_Size;
	public:
		OpenGLShaderUniformBufferDeclaration(const std::string& name);

		void PushUniform(OpenGLShaderUniformDeclaration* uniform);

		inline const std::string& GetName() const override { return m_Name; }
		inline uint32_t GetRegister() const override { return m_Register; }
		inline uint32_t GetSize() const override { return m_Size; }
		inline const ShaderUniformList& GetUniformDeclarations() const override { return m_Uniforms; }

		ShaderUniformDeclaration* FindUniform(const std::string& name);
	};
}