#pragma once

#include "Prism/Renderer/Buffer/BufferData.h"

#include <glad/glad.h>

typedef unsigned int GLenum;

namespace Prism
{
    inline GLenum OpenGLUsage(BufferUsage usage)
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
