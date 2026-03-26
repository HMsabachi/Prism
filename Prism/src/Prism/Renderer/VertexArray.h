#pragma once

#include <memory>
#include "Prism/Core/Core.h"
#include "Prism/Renderer/Buffer.h"

namespace Prism
{
	class PRISM_API VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;
	
	public:
		static VertexArray* Create();
	};
}
