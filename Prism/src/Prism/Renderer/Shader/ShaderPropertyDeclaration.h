#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Utilities/Utilities.h"
#include "Prism/Renderer/Texture.h"
#include <vector>

namespace Prism
{
	enum class PropertyDeclarationType : uint32_t
	{
		None,
		Bool,
		Color, Color3, Float, Int,
		Vector2, Vector3, Vector4,
		Range,
		Matrix3, Matrix4,
		Texture2D, Texture2DMS, TextureCube,
		Enum
	};

	namespace PropertyType
	{
		typedef Type::Bool Bool;
		typedef Type::Color Color;
		typedef Type::Vector3 Color3;
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
			uint32_t slot = 0;
			Ref<Prism::Texture2D> texture;
			explicit operator bool() const;
			operator Ref<Prism::Texture2D>() const;
			operator uint32_t() const;
			void SetTexture(Ref<Prism::Texture2D>& t);
			std::string ToString() const;
		};
		struct PRISM_API TextureCube
		{
			uint32_t slot = 0;
			Ref<Prism::TextureCube> texture;
			explicit operator bool() const;
			operator Ref<Prism::TextureCube>() const;
			operator uint32_t() const;
			void SetTexture(Ref<Prism::TextureCube>& t);
			std::string ToString() const;
		};
	}

	class PRISM_API PropertyDeclaration
	{
		friend class PropertyBufferDeclaration;
	public:
		PropertyDeclaration(PropertyDeclarationType type, const std::string& name, const std::string displayName, uint32_t count = 1);

		const std::string& GetName() const { return m_Name; }
		const std::string& GetDisplayName() const { return m_DisplayName; }
		uint32_t GetSize() const { return m_Size; }
		uint32_t GetCount() const { return m_Count; }
		uint32_t GetOffset() const { return m_Offset; }
		PropertyDeclarationType GetType() const { return m_Type; }
		bool IsArray() const { return m_Count > 1; }
		const std::vector<std::string>& GetEnumOptions() const { return m_EnumOptions; }
		void SetEnumOptions(const std::vector<std::string>& options) { m_EnumOptions = options; }

		template<typename T>
		T& GetValue(const Buffer& buffer) const { return *reinterpret_cast<T*>(buffer.Data + m_Offset); }

	protected:
		void SetOffset(uint32_t offset) { m_Offset = offset; }

	private:
		std::string m_Name;
		std::string m_DisplayName;
		uint32_t m_Size = 0;
		uint32_t m_Count = 1;
		uint32_t m_Offset = 0;
		PropertyDeclarationType m_Type = PropertyDeclarationType::None;
		std::vector<std::string> m_EnumOptions;

		static uint32_t SizeOfType(PropertyDeclarationType type);
	};

	class PRISM_API PropertyBufferDeclaration
	{
	public:
		PropertyBufferDeclaration() = default;

		PropertyDeclaration& AddProperty(const PropertyDeclaration& property);
		const PropertyDeclaration* FindProperty(const std::string& name) const;
		uint32_t GetSize() const { return m_Size; }
		bool IsEmpty() const { return m_Properties.empty(); }

		using const_iterator = std::vector<PropertyDeclaration>::const_iterator;
		const_iterator begin() const { return m_Properties.begin(); }
		const_iterator end() const { return m_Properties.end(); }

	private:
		std::vector<PropertyDeclaration> m_Properties;
		uint32_t m_Size = 0;
	};
}
