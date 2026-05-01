#pragma once
#include "prpch.h"

namespace Prism
{
	class PRISM_API RenderCommandQueue
	{
	private:
		static const size_t COMMAND_BUFFER_SIZE;
	public:
		typedef void(*RenderCommandFn)(void*);

		RenderCommandQueue();
		~RenderCommandQueue();

		void* Allocate(RenderCommandFn fn, uint32_t size);
		void Execute();

		// Debug
		void ResetSubmitCount() { m_SubmitCount = 0; };
		uint32_t GetSubmitCount() const { return m_SubmitCount; };
	private:
		uint8_t* m_CommandBuffer;
		uint8_t* m_CommandBufferPtr;
		uint32_t m_CommandCount;
		bool m_IsExecuting = false;
		uint32_t m_SubmitCount;
	};
}