#include "prpch.h"
#include "RenderCommandQueue.h"

#define PR_RENDER_TRACE(...) PR_CORE_TRACE(__VA_ARGS__)
namespace Prism
{

	const unsigned int RenderCommandQueue::COMMAND_BUFFER_SIZE = 10 * 1024 * 1024;

	RenderCommandQueue::RenderCommandQueue()
		:m_RenderCommandCount(0)
	{
		m_CommandBuffer = new unsigned char[COMMAND_BUFFER_SIZE]; // 10MB buffer
		m_CommandBufferPtr = m_CommandBuffer;
		memset(m_CommandBuffer, 0, COMMAND_BUFFER_SIZE);
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] m_CommandBuffer;
	}

	void RenderCommandQueue::Submit(const RenderCommand& command)
	{
		auto ptr = m_CommandBuffer;

		memcpy(m_CommandBuffer, &command, sizeof(RenderCommand));
		m_CommandBufferPtr += sizeof(RenderCommand);
		m_RenderCommandCount++;
	}
	void RenderCommandQueue::SubmitCommand(const RenderCommandFn& fn, void* params, unsigned int size)
	{
		byte*& buffer = m_CommandBufferPtr;
		memcpy(buffer, &fn, sizeof(RenderCommandFn));
		buffer += sizeof(RenderCommandFn);
		memcpy(buffer, params, size);
		buffer += size;

		// TODO: 这里的对齐有问题
		auto totalSize = sizeof(RenderCommandFn) + size;
		auto padding = (16 - totalSize % 16); // 16-byte alignment
		buffer += padding;

		m_RenderCommandCount++;
	}

	void RenderCommandQueue::Execute()
	{
		PR_RENDER_TRACE("RenderCommandQueue::Execute -- {0} commands, {1} bytes", m_RenderCommandCount, (m_CommandBufferPtr - m_CommandBuffer));

		byte* buffer = m_CommandBuffer;

		for (int i = 0; i < m_RenderCommandCount; i++)
		{
			RenderCommandFn fn = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);
			buffer += (*fn)(buffer);

			auto padding = (int)(16 - (int)buffer % 16);
			buffer += padding;
		}

		m_CommandBufferPtr = m_CommandBuffer;
		m_RenderCommandCount = 0;
	}
	

}