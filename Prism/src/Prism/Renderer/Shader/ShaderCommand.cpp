#include "prpch.h"
#include "ShaderCommand.h"

namespace Prism
{
	CullMode StringToCullMode(const std::string& s)
	{
		if (s == "Back")  return CullMode::Back;
		if (s == "Front") return CullMode::Front;
		if (s == "Off")   return CullMode::Off; 
		return CullMode::Back; 
	}

	DepthCompareFunc StringToDepthCompareFunc(const std::string& s)
	{
		if (s == "Never")        return DepthCompareFunc::Never;
		if (s == "Less")         return DepthCompareFunc::Less;
		if (s == "Equal")        return DepthCompareFunc::Equal;
		if (s == "LEqual" || s == "LessEqual") return DepthCompareFunc::LessEqual;
		if (s == "Greater")      return DepthCompareFunc::Greater;
		if (s == "NotEqual")     return DepthCompareFunc::NotEqual;
		if (s == "GEqual" || s == "GreaterEqual") return DepthCompareFunc::GreaterEqual;
		if (s == "Always")       return DepthCompareFunc::Always;
		return DepthCompareFunc::LessEqual;
	}

	BlendFactor StringToBlendFactor(const std::string& s)
	{
		if (s == "Zero")             return BlendFactor::Zero;
		if (s == "One")              return BlendFactor::One;
		if (s == "SrcAlpha")         return BlendFactor::SrcAlpha;
		if (s == "OneMinusSrcAlpha") return BlendFactor::OneMinusSrcAlpha;
		if (s == "DstAlpha")         return BlendFactor::DstAlpha;
		if (s == "OneMinusDstAlpha") return BlendFactor::OneMinusDstAlpha;
		return BlendFactor::One;
	}

	void ParseBlendCommand(StrParse::Tokens& tokens, ShaderCommand& state)
	{
		tokens.pop();
		std::string& value = tokens.front();
		if(value == "Off")
		{
			state.flags &= ~ShaderCommandFlag::Blend;
			tokens.pop();
			return;
		}
		if (tokens.size() >= 2) 
		{
			state.blendSrcRGB = StringToBlendFactor(tokens.front()); tokens.pop();
			state.blendDstRGB = StringToBlendFactor(tokens.front()); tokens.pop();
			if (tokens.size() >= 2) 
			{
				state.blendSrcAlpha = StringToBlendFactor(tokens.front());  tokens.pop();
				state.blendDstAlpha = StringToBlendFactor(tokens.front());  tokens.pop();
			}
			else 
			{
				state.blendSrcAlpha = state.blendSrcRGB;
				state.blendDstAlpha = state.blendDstRGB;
			}
			state.flags |= ShaderCommandFlag::Blend;
		}
		
	}
	void ParseCullCommand(StrParse::Tokens& tokens, ShaderCommand& state)
	{
		tokens.pop();
		if (!tokens.empty())
		{
			state.cullMode = StringToCullMode(tokens.front());
			if(state.cullMode != CullMode::Off)
				state.flags |= ShaderCommandFlag::Cull;
			tokens.pop();
		}
	}
	void ParseZTestCommand(StrParse::Tokens& tokens, ShaderCommand& state)
	{
		tokens.pop();
		if (tokens.front() == "Off")
		{
			state.flags &= ~ShaderCommandFlag::ZTest;
			tokens.pop();
			return;
		}
		if (!tokens.empty())
		{
			state.zTestFunc = StringToDepthCompareFunc(tokens.front());
			tokens.pop();
		}
	}
	void ParseZWriteCommand(StrParse::Tokens& tokens, ShaderCommand& state)
	{
		tokens.pop();
		if (!tokens.empty())
		{
			std::string& value = tokens.front();
			if (value == "On")
				state.flags |= ShaderCommandFlag::ZWrite;
			else if (value == "Off")
				state.flags &= ~ShaderCommandFlag::ZWrite;
			tokens.pop();
		}
	}

	ShaderCommand ParseShaderCommand(const std::string& commandStr)
	{
		using namespace StrParse;
		TokenLines tokenLines;
		StrParse::SplitToken(commandStr, tokenLines);
		ShaderCommand state;
		while (!tokenLines.empty())
		{
			Tokens tokens = tokenLines.front();
			std::string& tokenKey = tokens.front();
			if (tokenKey == "Blend")
				ParseBlendCommand(tokens, state);
			else if (tokenKey == "Cull")
				ParseCullCommand(tokens, state);
			else if (tokenKey == "ZTest")
				ParseZTestCommand(tokens, state);
			else if (tokenKey == "ZWrite")
				ParseZWriteCommand(tokens, state);
			tokenLines.pop();
		}
		return state;
	}

}