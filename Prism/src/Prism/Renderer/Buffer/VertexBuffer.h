#pragma once
#include "BufferData.h"
#include <PrismShaderCore/Property/VertexType.h>

#include "../RendererAPI.h"

namespace Prism {
    using PrismShaderCompiler::VertexSemantic;

#pragma region 布局相关

	enum class PRISM_API ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};
	uint32_t ShaderDataTypeSize(ShaderDataType type);

	struct PRISM_API VertexBufferElement
	{
		static const uint32_t DEFAULT_VERTEX_SEMANTICS = 10;

		std::string Name;
		ShaderDataType Type = ShaderDataType::None;
		VertexSemantic Semantic = VertexSemantic::Unknown;
		uint32_t Size = 0;
		uint32_t Offset = 0;
		bool Normalized = false;

		VertexBufferElement() = default;

		VertexBufferElement(ShaderDataType type, const std::string& name, VertexSemantic semantic = VertexSemantic::Unknown, bool normalized = false)
			: Name(name), Type(type), Semantic(semantic), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}
		uint32_t GetIndex() const { return Semantic == VertexSemantic::Unknown ? DEFAULT_VERTEX_SEMANTICS : (uint32_t)Semantic; }

		uint32_t GetComponentCount() const;
	};

	class PRISM_API VertexBufferLayout
	{
	public:
		VertexBufferLayout() {}

		VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsStrideHash();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
        inline uint64_t GetHash() const { return m_Hash; }

		std::vector<VertexBufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<VertexBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsStrideHash()
		{
            constexpr uint64_t FNV_PRIME = 1099511628211ULL;
            constexpr uint64_t OFFSET_BASIS = 14695981039346656037ULL;
			uint32_t offset = 0;
			m_Stride = 0;
            m_Hash = OFFSET_BASIS;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
                m_Hash ^= static_cast<uint64_t>(element.Type); m_Hash *= FNV_PRIME;
                m_Hash ^= static_cast<uint64_t>(element.Semantic); m_Hash *= FNV_PRIME;
                m_Hash ^= static_cast<uint64_t>(element.Size); m_Hash *= FNV_PRIME;
                m_Hash ^= static_cast<uint64_t>(element.Offset); m_Hash *= FNV_PRIME;
                m_Hash ^= static_cast<uint64_t>(element.Normalized); m_Hash *= FNV_PRIME;
			}
            m_Hash ^= static_cast<uint64_t>(m_Stride); m_Hash *= FNV_PRIME;
		}
	private:
		std::vector<VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;
        uint64_t m_Hash = 0;
	};

#pragma endregion

	class PRISM_API VertexBuffer : public RefCounted
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void SetData(void* buffer, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Bind() const = 0;

		virtual const VertexBufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const VertexBufferLayout& layout) = 0;

		virtual uint32_t GetSize() const = 0;

		static Ref<VertexBuffer> Create(void* data, uint32_t size = 0, BufferUsage usage = BufferUsage::Dynamic);
		static Ref<VertexBuffer> Create(uint32_t size = 0, BufferUsage usage = BufferUsage::Dynamic);
	};

}
