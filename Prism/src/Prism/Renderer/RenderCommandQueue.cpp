#include "prpch.h"
#include "RenderCommandQueue.h"

#define PR_RENDER_TRACE(...) PR_CORE_TRACE(__VA_ARGS__)
namespace Prism
{

	const size_t RenderCommandQueue::COMMAND_BUFFER_SIZE = 10 * 1024 * 1024;

	RenderCommandQueue::RenderCommandQueue()
		:m_CommandCount(0)
	{
		m_CommandBuffer = new uint8_t[COMMAND_BUFFER_SIZE]; // 10MB buffer
		m_CommandBufferPtr = m_CommandBuffer;
		memset(m_CommandBuffer, 0, COMMAND_BUFFER_SIZE);
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] m_CommandBuffer;
	}

	void* RenderCommandQueue::Allocate(RenderCommandFn fn, uint32_t size)
	{
		if(m_IsExecuting) 
			PR_CORE_WARN("RenderCommandQueue: 在执行命令队列时分配新了的命令!");
		// TODO: 对齐 alignment
		*(RenderCommandFn*)m_CommandBufferPtr = fn;
		m_CommandBufferPtr += sizeof(RenderCommandFn);

		*(uint32_t*)m_CommandBufferPtr = size;
		m_CommandBufferPtr += sizeof(uint32_t);

		void* memory = m_CommandBufferPtr;
		m_CommandBufferPtr += size;

		m_CommandCount++;
		return memory;
	}

	void RenderCommandQueue::Execute()
	{
		PR_PROFILE_FUNCTION();
		m_IsExecuting = true;

		//PR_RENDER_TRACE("RenderCommandQueue::Execute -- {0} commands, {1} bytes", m_CommandCount, (m_CommandBufferPtr - m_CommandBuffer));

		byte* buffer = m_CommandBuffer;

		for (uint32_t i = 0; i < m_CommandCount; i++)
		{
			RenderCommandFn function = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);

			uint32_t size = *(uint32_t*)buffer;
			buffer += sizeof(uint32_t);
			function(buffer);
			buffer += size;
		}

		m_CommandBufferPtr = m_CommandBuffer;
		m_CommandCount = 0;
		m_IsExecuting = false;
	}
	

}