#pragma once
#include "Prism/Renderer/Renderer.h"

namespace Prism {

	class PRISM_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void SetData(void* buffer, unsigned int size, unsigned int offset = 0) = 0;
		virtual void Bind() const = 0;

		virtual unsigned int GetSize() const = 0;
		virtual RendererID GetRendererID() const = 0;

		static VertexBuffer* Create(unsigned int size = 0);
	};

	class PRISM_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void SetData(void* buffer, unsigned int size, unsigned int offset = 0) = 0;
		virtual void Bind() const = 0;

		virtual unsigned int GetSize() const = 0;
		virtual RendererID GetRendererID() const = 0;

		static IndexBuffer* Create(unsigned int size = 0);
	};

}