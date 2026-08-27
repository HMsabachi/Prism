#include "prpch.h"
#include "OpenGLPipelineState.h"

#include "Prism/Renderer/Renderer.h"
#include <glad/glad.h>

using namespace PrismShaderCompiler;

namespace Prism
{
#pragma region 转换OpenGL类型
    static GLenum ToOpenGL(CullMode mode)
    {
        switch (mode)
        {
        case CullMode::Back:  return GL_BACK;
        case CullMode::Front: return GL_FRONT;
        case CullMode::Off:   return GL_BACK;
        }
        return GL_BACK;
    }

    static GLenum ToOpenGL(DepthFunc func)
    {
        switch (func)
        {
        case DepthFunc::Never:    return GL_NEVER;
        case DepthFunc::Less:     return GL_LESS;
        case DepthFunc::Equal:    return GL_EQUAL;
        case DepthFunc::LEqual:   return GL_LEQUAL;
        case DepthFunc::Greater:  return GL_GREATER;
        case DepthFunc::NotEqual: return GL_NOTEQUAL;
        case DepthFunc::GEqual:   return GL_GEQUAL;
        case DepthFunc::Always:   return GL_ALWAYS;
        }
        return GL_LEQUAL;
    }

    static GLenum ToOpenGL(BlendFactor factor)
    {
        switch (factor)
        {
        case BlendFactor::Zero:             return GL_ZERO;
        case BlendFactor::One:              return GL_ONE;
        case BlendFactor::SrcAlpha:         return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:         return GL_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
        }
        return GL_ONE;
    }

    static GLenum ToOpenGL(StencilFunc func)
    {
        switch (func)
        {
        case StencilFunc::Never:    return GL_NEVER;
        case StencilFunc::Less:     return GL_LESS;
        case StencilFunc::Equal:    return GL_EQUAL;
        case StencilFunc::LEqual:   return GL_LEQUAL;
        case StencilFunc::Greater:  return GL_GREATER;
        case StencilFunc::NotEqual: return GL_NOTEQUAL;
        case StencilFunc::GEqual:   return GL_GEQUAL;
        case StencilFunc::Always:   return GL_ALWAYS;
        }
        return GL_ALWAYS;
    }

    static GLenum ToOpenGL(StencilOp op)
    {
        switch (op)
        {
        case StencilOp::Keep:     return GL_KEEP;
        case StencilOp::Zero:     return GL_ZERO;
        case StencilOp::Replace:  return GL_REPLACE;
        case StencilOp::Incr:     return GL_INCR;
        case StencilOp::IncrWrap: return GL_INCR_WRAP;
        case StencilOp::Decr:     return GL_DECR;
        case StencilOp::DecrWrap: return GL_DECR_WRAP;
        case StencilOp::Invert:   return GL_INVERT;
        }
        return GL_KEEP;
    }

    static GLenum ToOpenGL(PolygonMode mode)
    {
        switch (mode)
        {
        case PolygonMode::Fill:  return GL_FILL;
        case PolygonMode::Line:  return GL_LINE;
        case PolygonMode::Point: return GL_POINT;
        }
        return GL_FILL;
    }
#pragma endregion


    void OpenGLPipelineState::RT_SetupPipelineState(const PrismShaderCompiler::PipelineState& state)
    {
        if (s_CurrentHash == state.Hash)
            return;
        s_CurrentHash = state.Hash;
        if (state.BlendEnabled)
        {
            glEnable(GL_BLEND);
            glBlendFuncSeparate(
                ToOpenGL(state.SrcFactor), ToOpenGL(state.DstFactor),
                ToOpenGL(state.SrcAlpha), ToOpenGL(state.DstAlpha));
        }
        else
        {
            glDisable(GL_BLEND);
        }
        if (state.Cull == CullMode::Off)
            glDisable(GL_CULL_FACE);
        else
        {
            glEnable(GL_CULL_FACE);
            glCullFace(ToOpenGL(state.Cull));
        }

        if (state.DepthTest)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(ToOpenGL(state.DepthCompare));
        }
        else
            glDisable(GL_DEPTH_TEST);

        glDepthMask(state.DepthWrite ? GL_TRUE : GL_FALSE);

        {
            GLboolean r = GL_FALSE, g = GL_FALSE, b = GL_FALSE, a = GL_FALSE;
            switch (state.WriteMask)
            {
            case ColorMask::RGBA: r = g = b = a = GL_TRUE; break;
            case ColorMask::RGB:  r = g = b = GL_TRUE;     break;
            case ColorMask::R:    r = GL_TRUE;             break;
            case ColorMask::G:    g = GL_TRUE;             break;
            case ColorMask::B:    b = GL_TRUE;             break;
            case ColorMask::A:    a = GL_TRUE;             break;
            case ColorMask::None: break;
            }
            glColorMask(r, g, b, a);
        }

        {
            bool needBias = state.DepthBiasFactor != 0.0f || state.DepthBiasUnits != 0.0f;
            if (needBias)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(state.DepthBiasFactor, state.DepthBiasUnits);
            }
            else
                glDisable(GL_POLYGON_OFFSET_FILL);
        }

        if (state.StencilTest)
        {
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(ToOpenGL(state.StencilCompare), state.StencilRef, state.StencilReadMask);
            glStencilOp(ToOpenGL(state.StencilFailOp), ToOpenGL(state.StencilDepthFailOp), ToOpenGL(state.StencilPassOp));
            glStencilMask(state.StencilWriteMask);
        }
        else
            glDisable(GL_STENCIL_TEST);

        glPolygonMode(GL_FRONT_AND_BACK, ToOpenGL(state.FillMode));

        glLineWidth(state.LineWidth);
    }

}
