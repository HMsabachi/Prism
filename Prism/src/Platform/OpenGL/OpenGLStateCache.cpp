#include "prpch.h"
#include "OpenGLStateCache.h"
#include "Prism/Renderer/Shader/ShaderCommand.h"  

#include <glad/glad.h>

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

	static GLenum ToOpenGL(DepthCompareFunc func)
	{
		switch (func)
		{
		case DepthCompareFunc::Never:        return GL_NEVER;
		case DepthCompareFunc::Less:         return GL_LESS;
		case DepthCompareFunc::Equal:        return GL_EQUAL;
		case DepthCompareFunc::LessEqual:    return GL_LEQUAL;
		case DepthCompareFunc::Greater:      return GL_GREATER;
		case DepthCompareFunc::NotEqual:     return GL_NOTEQUAL;
		case DepthCompareFunc::GreaterEqual: return GL_GEQUAL;
		case DepthCompareFunc::Always:       return GL_ALWAYS;
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

	static GLenum ToOpenGL(BlendOperation op)
	{
		switch (op)
		{
		case BlendOperation::Add:             return GL_FUNC_ADD;
		case BlendOperation::Subtract:        return GL_FUNC_SUBTRACT;
		case BlendOperation::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
		case BlendOperation::Min:             return GL_MIN;
		case BlendOperation::Max:             return GL_MAX;
		}
		return GL_FUNC_ADD;
	}
#pragma endregion



	static ShaderCommand s_Current;

	void OpenGLStateCache::Init()
	{
		Reset();   // 初始化为 OpenGL 默认状态
	}

	void OpenGLStateCache::Reset()
	{

		s_Current = ShaderCommand();  

		// 把 OpenGL 重置到默认状态
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
	}



	void OpenGLStateCache::Apply(const ShaderCommand& newCommand)
	{
		PR_PROFILE_FUNCTION();

		ApplyBlendIfChanged(newCommand);
		ApplyCullIfChanged(newCommand);
		ApplyDepthIfChanged(newCommand);
		ApplyZWriteIfChanged(newCommand);

		// 更新缓存
		s_Current = newCommand;
	}



	void OpenGLStateCache::ApplyBlendIfChanged(const ShaderCommand& cmd)
	{
		bool needBlend = cmd.flags.Has(ShaderCommandFlag::Blend);
		bool currBlend = s_Current.flags.Has(ShaderCommandFlag::Blend);

		if (needBlend != currBlend)
		{
			if (needBlend)
			{
				glEnable(GL_BLEND);
				glBlendFuncSeparate(
					ToOpenGL(cmd.blendSrcRGB), ToOpenGL(cmd.blendDstRGB),
					ToOpenGL(cmd.blendSrcAlpha), ToOpenGL(cmd.blendDstAlpha)
				);
				glBlendEquationSeparate(
					ToOpenGL(cmd.blendOpRGB), ToOpenGL(cmd.blendOpAlpha)
				);
			}
			else
			{
				glDisable(GL_BLEND);
			}
			return;
		}


		if (needBlend &&
			(cmd.blendSrcRGB != s_Current.blendSrcRGB ||
				cmd.blendDstRGB != s_Current.blendDstRGB ||
				cmd.blendSrcAlpha != s_Current.blendSrcAlpha ||
				cmd.blendDstAlpha != s_Current.blendDstAlpha ||
				cmd.blendOpRGB != s_Current.blendOpRGB ||
				cmd.blendOpAlpha != s_Current.blendOpAlpha))
		{
			glBlendFuncSeparate(
				ToOpenGL(cmd.blendSrcRGB), ToOpenGL(cmd.blendDstRGB),
				ToOpenGL(cmd.blendSrcAlpha), ToOpenGL(cmd.blendDstAlpha)
			);
			glBlendEquationSeparate(
				ToOpenGL(cmd.blendOpRGB), ToOpenGL(cmd.blendOpAlpha)
			);
		}
	}

	void OpenGLStateCache::ApplyCullIfChanged(const ShaderCommand& cmd)
	{
		bool needCull = cmd.flags.Has(ShaderCommandFlag::Cull);
		bool currCull = s_Current.flags.Has(ShaderCommandFlag::Cull);

		if (needCull != currCull)
		{
			if (needCull)
			{
				glEnable(GL_CULL_FACE);
				glCullFace(ToOpenGL(cmd.cullMode));
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}
			return;
		}

		if (needCull && cmd.cullMode != s_Current.cullMode)
		{
			glCullFace(ToOpenGL(cmd.cullMode));
		}
	}

	void OpenGLStateCache::ApplyDepthIfChanged(const ShaderCommand& cmd)
	{
		bool needZTest = cmd.flags.Has(ShaderCommandFlag::ZTest);
		bool currZTest = s_Current.flags.Has(ShaderCommandFlag::ZTest);

		if (needZTest != currZTest)
		{
			if (needZTest)
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(ToOpenGL(cmd.zTestFunc));
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}
			return;
		}

		if (needZTest && cmd.zTestFunc != s_Current.zTestFunc)
		{
			glDepthFunc(ToOpenGL(cmd.zTestFunc));
		}
	}

	void OpenGLStateCache::ApplyZWriteIfChanged(const ShaderCommand& cmd)
	{
		bool needZWrite = cmd.flags.Has(ShaderCommandFlag::ZWrite);
		bool currZWrite = s_Current.flags.Has(ShaderCommandFlag::ZWrite);

		if (needZWrite != currZWrite)
		{
			glDepthMask(needZWrite ? GL_TRUE : GL_FALSE);
		}
	}
}