#pragma once
#include "prpch.h"

namespace Prism
{
	class PRISM_API RenderCommandQueue
	{
	private:
		static const unsigned int COMMAND_BUFFER_SIZE;
	public:
		using RenderCommand = std::function<unsigned int* (void*)>;
		typedef unsigned int (*RenderCommandFn)(void*);

		RenderCommandQueue();
		~RenderCommandQueue();

		void Submit(const RenderCommand& command);
		void SubmitCommand(const RenderCommandFn& fn, void* params, unsigned int size);
		void Execute();

	private:
		unsigned char* m_CommandBuffer;
		unsigned char* m_CommandBufferPtr;
		unsigned int m_RenderCommandCount;
	};
}