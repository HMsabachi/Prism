#include "prpch.h"
#include "OpenGLStateCache.h"
#include <PrismShaderCore/Pipeline/PipelineState.h>

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
#pragma endregion

    static PipelineState s_Current;

    void OpenGLStateCache::Init()
    {
        Reset();
    }

    void OpenGLStateCache::Reset()
    {
        s_Current = PipelineState::Default();

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    void OpenGLStateCache::Apply(const PipelineState& newCommand)
    {
        ApplyBlendIfChanged(newCommand);
        ApplyCullIfChanged(newCommand);
        ApplyDepthIfChanged(newCommand);
        ApplyColorMaskIfChanged(newCommand);
        ApplyDepthBiasIfChanged(newCommand);

        s_Current = newCommand;
    }

    void OpenGLStateCache::ApplyBlendIfChanged(const PipelineState& cmd)
    {
        if (cmd.BlendEnabled != s_Current.BlendEnabled)
        {
            if (cmd.BlendEnabled)
            {
                glEnable(GL_BLEND);
                glBlendFuncSeparate(
                    ToOpenGL(cmd.SrcFactor), ToOpenGL(cmd.DstFactor),
                    ToOpenGL(cmd.SrcAlpha), ToOpenGL(cmd.DstAlpha)
                );
            }
            else
            {
                glDisable(GL_BLEND);
            }
            return;
        }

        if (cmd.BlendEnabled &&
            (cmd.SrcFactor != s_Current.SrcFactor ||
                cmd.DstFactor != s_Current.DstFactor ||
                cmd.SrcAlpha != s_Current.SrcAlpha ||
                cmd.DstAlpha != s_Current.DstAlpha))
        {
            glBlendFuncSeparate(
                ToOpenGL(cmd.SrcFactor), ToOpenGL(cmd.DstFactor),
                ToOpenGL(cmd.SrcAlpha), ToOpenGL(cmd.DstAlpha)
            );
        }
    }

    void OpenGLStateCache::ApplyCullIfChanged(const PipelineState& cmd)
    {
        if (cmd.Cull != s_Current.Cull)
        {
            if (cmd.Cull == CullMode::Off)
            {
                glDisable(GL_CULL_FACE);
            }
            else
            {
                glEnable(GL_CULL_FACE);
                glCullFace(ToOpenGL(cmd.Cull));
            }
        }
    }

    void OpenGLStateCache::ApplyDepthIfChanged(const PipelineState& cmd)
    {
        if (cmd.DepthTest != s_Current.DepthTest)
        {
            if (cmd.DepthTest)
            {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(ToOpenGL(cmd.DepthCompare));
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }
        }
        else if (cmd.DepthTest && cmd.DepthCompare != s_Current.DepthCompare)
        {
            glDepthFunc(ToOpenGL(cmd.DepthCompare));
        }

        if (cmd.DepthWrite != s_Current.DepthWrite)
        {
            glDepthMask(cmd.DepthWrite ? GL_TRUE : GL_FALSE);
        }
    }

    void OpenGLStateCache::ApplyColorMaskIfChanged(const PipelineState& cmd)
    {
        if (cmd.WriteMask != s_Current.WriteMask)
        {
            GLboolean r = GL_FALSE, g = GL_FALSE, b = GL_FALSE, a = GL_FALSE;
            switch (cmd.WriteMask)
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
    }

    void OpenGLStateCache::ApplyDepthBiasIfChanged(const PipelineState& cmd)
    {
        bool needBias = cmd.DepthBiasFactor != 0.0f || cmd.DepthBiasUnits != 0.0f;
        bool currBias = s_Current.DepthBiasFactor != 0.0f || s_Current.DepthBiasUnits != 0.0f;

        if (needBias != currBias)
        {
            if (needBias)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(cmd.DepthBiasFactor, cmd.DepthBiasUnits);
            }
            else
            {
                glDisable(GL_POLYGON_OFFSET_FILL);
            }
        }
        else if (needBias &&
                 (cmd.DepthBiasFactor != s_Current.DepthBiasFactor ||
                     cmd.DepthBiasUnits != s_Current.DepthBiasUnits))
        {
            glPolygonOffset(cmd.DepthBiasFactor, cmd.DepthBiasUnits);
        }
    }
}
