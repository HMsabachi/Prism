#pragma once
#include <string>
#include <set>
#include <filesystem>
#include "ShaderParserData.h"

namespace Prism
{
	class ShaderParser
	{
	public:
		static void Init(const std::string& includeRoot);

		ParseResult Parse(const std::string& source);

	private:
		static PropertyDeclarationType StringToPropertyType(const std::string& typeStr, float& outMin, float& outMax);
		static std::string PropertyTypeToGLSL(PropertyDeclarationType type);
		static int GetLocationBySemantic(const std::string& semantic, VertexSemantic& outSemantic);

		static std::string StripComments(const std::string& source);
		static bool ExtractShaderMetadata(const std::string& source, ParseResult& outResult);
		static bool ParsePropertiesBlock(const std::string& source, ParseResult& outResult);
		static bool ParseRenderCommand(const std::string& source, ParseResult& result);
		static bool ParseSubShader(const std::string& source, ParseResult& outResult);
		static void ParsePassInternal(const std::string& passContent, PassDescriptor& outPass);
		static std::string ResolveIncludes(const std::string& source, const std::filesystem::path& includeRoot, std::set<std::filesystem::path>& includeHistory);
		static void ProcessAttributes(PassDescriptor& pass);
		static void InjectHeader(PassDescriptor& pass, const ParseResult& result);
		static void SplitShader(PassDescriptor& pass);
		static void RemoveFunction(std::string& code, const std::string& funcName);
		static void FormatCodeInPlace(std::string& code);

		static std::string s_IncludeRoot;
		static std::string s_VersionHeader;
	};
}
