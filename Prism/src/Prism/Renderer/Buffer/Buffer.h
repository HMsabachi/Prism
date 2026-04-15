#pragma once
#include "PrismShaderData.h"
#include "BufferData.h"

#include "../RendererAPI.h"

namespace Prism {

#pragma region 布局相关

	enum class PRISM_API ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};
	uint32_t ShaderDataTypeSize(ShaderDataType type);

	struct PRISM_API BufferElement
	{
		static const uint32_t DEFAULT_VERTEX_SEMANTICS = 10;

		std::string Name;
		ShaderDataType Type;
		VertexSemantic Semantic;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() = default;

		BufferElement(ShaderDataType type, const std::string& name, VertexSemantic semantic = VertexSemantic::Unknown, bool normalized = false)
			: Name(name), Type(type), Semantic(semantic), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}
		uint32_t GetIndex() const { return Semantic == VertexSemantic::Unknown ? DEFAULT_VERTEX_SEMANTICS : (uint32_t)Semantic; }

		uint32_t GetComponentCount() const;
	};

	class PRISM_API BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

#pragma endregion

	class PRISM_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void SetData(void* buffer, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Bind() const = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		virtual uint32_t GetSize() const = 0;
		virtual RendererID GetRendererID() const = 0;

		static Ref<VertexBuffer> Create(void* data, uint32_t size = 0, BufferUsage usage = BufferUsage::Dynamic);
		static Ref<VertexBuffer> Create(uint32_t size = 0, BufferUsage usage = BufferUsage::Dynamic);
	};

	class PRISM_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void SetData(void* buffer, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Bind() const = 0;

		virtual uint32_t GetSize() const = 0;
		virtual uint32_t GetCount() const = 0;
		virtual RendererID GetRendererID() const = 0;

		static Ref<IndexBuffer> Create(void* data, uint32_t size = 0);
	};

}