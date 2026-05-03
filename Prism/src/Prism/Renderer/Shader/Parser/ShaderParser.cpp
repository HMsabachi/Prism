#include "prpch.h"
#include "ShaderParser.h"
#include "Lexer.h"
#include "Parser.h"
#include "CodeGen.h"
#include <unordered_set>

namespace Prism
{
	std::string ShaderParser::s_VersionHeader = "#version 450 core\n";
	std::string ShaderParser::s_IncludeRoot = "Assets/Shaders/Include";

	void ShaderParser::Init(const std::string& includeRoot)
	{
		s_IncludeRoot = std::filesystem::absolute(includeRoot).string();
	}

	// ======================================================================
	// Parse — New pipeline: Lexer → Parser → CodeGen
	// ======================================================================

	ParseResult ShaderParser::Parse(const std::string& source)
	{
		Lexer lexer(source);
		auto tokens = lexer.Tokenize();

		Parser parser(tokens, source);
		ParseResult result = parser.ParseShader();

		// Report lexer errors
		for (const auto& err : lexer.GetErrors())
			PR_CORE_ERROR("  Lexer error: Line {0}:{1}: {2}", err.Line, err.Column, err.Message);

		// Report parser errors
		for (const auto& err : parser.GetErrors())
			PR_CORE_ERROR("  Parsing error: Line {0}:{1}: {2}", err.Line, err.Column, err.Message);

		if (!result.Success)
			PR_CORE_ERROR("Shader parsing result marked as failed");

		// Run code generation if we have passes
		if (!result.Passes.empty())
		{
			CodeGen codegen(s_IncludeRoot);
			for (auto& pass : result.Passes)
				codegen.ProcessPass(pass, result.Properties);
		}

		// Aggregate unique keywords from all passes (Phase 7)
		std::unordered_set<std::string> uniqueKeywords;
		for (const auto& pass : result.Passes)
		{
			for (const auto& pragma : pass.GLSL.Pragmas)
			{
				for (const auto& kw : pragma.Keywords)
				{
					if (kw != "_")
						uniqueKeywords.insert(kw);
				}
			}
		}
		result.Keywords.assign(uniqueKeywords.begin(), uniqueKeywords.end());

		return result;
	}
}
