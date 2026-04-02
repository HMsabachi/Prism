#pragma once
#include "prpch.h"

namespace Prism
{
	class PRISM_API RenderCommandQueue
	{
	private:
		static const unsigned int COMMAND_BUFFER_SIZE;
	public:
		typedef void(*RenderCommandFn)(void*);

		RenderCommandQueue();
		~RenderCommandQueue();

		void* Allocate(RenderCommandFn fn, unsigned int size);
		void Execute();

	private:
		unsigned char* m_CommandBuffer;
		unsigned char* m_CommandBufferPtr;
		unsigned int m_CommandCount;
	};
}