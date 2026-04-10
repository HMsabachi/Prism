#pragma once
#include "../Texture.h"

// 前向声明
namespace Prism 
{
	struct PRISM_API PropertyDescriptor;
}

namespace Prism
{
	namespace PropertyType 
	{
		typedef Type::Color Color; // 暂时用vec4表示Color，后续可以改成专门的Color类型
		typedef Type::Float Float;
		typedef Type::Int Int;
		typedef Type::Vector2 Vector2;
		typedef Type::Vector3 Vector3;
		typedef Type::Vector4 Vector4;
		typedef Type::Range Range;
		typedef Type::Matrix3 Matrix3;
		typedef Type::Matrix4 Matrix4;
		struct PRISM_API Texture2D
		{
			Ref<Prism::Texture2D> texture;
			uint32_t slot = -1;
			explicit operator bool() const;
			operator Ref<Prism::Texture2D>() const;
			operator uint32_t() const;
			void SetTexture(Ref<Prism::Texture2D>& t);
			std::string ToString() const;
		};
		struct PRISM_API TextureCube
		{
			Ref<Prism::TextureCube> texture;
			uint32_t slot = -1;
			explicit operator bool() const;
			operator Ref<Prism::TextureCube>() const;
			operator uint32_t() const;
			void SetTexture(Ref<Prism::TextureCube>& t);
			std::string ToString() const;
		};
	}

#pragma region 描述器
	class PRISM_API PropertyDeclaration
	{
		friend class PropertyBufferDeclaration;
	public:
		enum class Type
		{
			None,
			Color, Float, Int,
			Vector2, Vector3, Vector4,
			Range,
			Matrix3, Matrix4,
			Texture2D, TextureCube
		};
	private:
		std::string m_Name;
		std::string m_DisplayName;
		uint32_t m_Size;
		uint32_t m_Count;
		uint32_t m_Offset;
		Type m_Type = Type::None;
	public:
		PropertyDeclaration(Type type, const std::string& name, const std::string displayName, uint32_t count = 1);

		const std::string& GetName() const  { return m_Name; }
		uint32_t GetSize() const  { return m_Size; }
		uint32_t GetCount() const  { return m_Count; }
		uint32_t GetOffset() const  { return m_Offset; }

		inline Type GetType() const { return m_Type; }
		inline bool IsArray() const { return m_Count > 1; }
	protected:
		void SetOffset(uint32_t offset)  { m_Offset = offset; }
	private:
		static uint32_t SizeOfType(Type type);
	};
	typedef std::vector<PropertyDeclaration*> PropertyList;

	class PRISM_API PropertyBufferDeclaration
	{
		friend class ShaderProperty;
	private:
		PropertyList m_Properties;
		uint32_t m_Size;
	public:
		PropertyBufferDeclaration();
		void Shutdown();
		void PushUniform(PropertyDeclaration* property);
		const PropertyDeclaration* FindUniform(const std::string& name) const;
		inline uint32_t GetSize() const { return m_Size; }
		inline const PropertyList& GetUniformDeclarations() const { return m_Properties; }

	public:
		const PropertyList::const_iterator begin() const { return m_Properties.begin(); }
		const PropertyList::const_iterator end() const { return m_Properties.end(); }
	private:
		PropertyList::iterator begin() { return m_Properties.begin(); }
		PropertyList::iterator end() { return m_Properties.end(); }
	};
#pragma endregion

	class PRISM_API ShaderProperty
	{
	public:
		ShaderProperty();
		void Init(const std::vector<PropertyDescriptor>& properties);

		const Buffer& GetDefaultValueBuffer() const;
		const PropertyBufferDeclaration& GetDeclaration() const;
	private:
		void HandlePropertyValue(const std::vector<PropertyDescriptor>& properties);
	private:
		bool m_IsLoaded = false;
		Buffer m_DefaultValueBuffer;
		PropertyBufferDeclaration m_Declaration;
	};
}