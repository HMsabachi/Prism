#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "GLSLParser.h"

namespace Prism
{
	enum class PropertyDeclarationType : uint32_t;
	enum class VertexSemantic
	{
		Position,
		Normal,
		Tangent,
		Binormal,
		TexCoord0,
		TexCoord1,
		BoneIndices,
		BoneWeights,
		InstanceID,
		Color,
		Index0,
		Index1,
		Other0,
		Other1,
		Other2,
		Unknown
	};

	struct VertexAttributeDescriptor
	{
		std::string Name;
		std::string Type;
		std::string SemanticStr;
		VertexSemantic Semantic;
		int Location = 0;
	};

	struct PropertyDescriptor
	{
		std::string Name;
		std::string DisplayName;
		PropertyDeclarationType Type;
		std::string DefaultValue;
		float Min = 0.0f;
		float Max = 1.0f;
		std::vector<std::string> EnumOptions;
	};

	struct PassDescriptor
	{
		std::string Name;
		std::unordered_map<std::string, std::string> Tags;
		std::vector<VertexAttributeDescriptor> Attributes;
		std::string RawGLSL;
		std::string VertexShaderCode;
		std::string FragmentShaderCode;
		size_t GLSLSourceLine = 0;

		// Phase 3: GLSL 结构化解析结果
		GLSLParseResult GLSL;
	};

	struct ParseResult
	{
		std::string ShaderName;
		std::vector<PropertyDescriptor> Properties;
		std::vector<PassDescriptor> Passes;
		std::string RenderCommand;
		bool Success = false;
	};
}
