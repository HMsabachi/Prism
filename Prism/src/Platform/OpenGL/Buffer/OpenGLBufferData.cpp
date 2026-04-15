#include "prpch.h"
#include "OpenGLBufferData.h"

#include "glad/glad.h"

namespace Prism
{
	GLenum OpenGLUsage(BufferUsage usage)
	{
		switch (usage)
		{
		case BufferUsage::Static:    return GL_STATIC_DRAW;
		case BufferUsage::Dynamic:   return GL_DYNAMIC_DRAW;
		}
		PR_CORE_ASSERT(false, "Unknown vertex buffer usage");
		return 0;
	}
}