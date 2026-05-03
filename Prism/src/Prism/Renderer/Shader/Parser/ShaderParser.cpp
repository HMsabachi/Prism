#include "prpch.h"
#include "ShaderParser.h"
#include "Prism/Renderer/Shader/ShaderPropertyDeclaration.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

namespace Prism
{
	std::string ShaderParser::s_VersionHeader = "#version 450 core\n";
	std::string ShaderParser::s_IncludeRoot = "Assets/Shaders/Include";

	void ShaderParser::Init(const std::string& includeRoot)
	{
		s_IncludeRoot = std::filesystem::absolute(includeRoot).string();
	}

	PropertyDeclarationType ShaderParser::StringToPropertyType(const std::string& typeStr, float& outMin, float& outMax)
	{
		if (typeStr == "Bool") return PropertyDeclarationType::Bool;
		if (typeStr == "Color") return PropertyDeclarationType::Color;
		if (typeStr == "Color3") return PropertyDeclarationType::Color3;
		if (typeStr == "Float") return PropertyDeclarationType::Float;
		if (typeStr == "Int")   return PropertyDeclarationType::Int;
		if (typeStr == "Vector2") return PropertyDeclarationType::Vector2;
		if (typeStr == "Vector3") return PropertyDeclarationType::Vector3;
		if (typeStr == "Vector4") return PropertyDeclarationType::Vector4;
		if (typeStr == "Texture2D") return PropertyDeclarationType::Texture2D;
		if (typeStr == "Texture2DMS") return PropertyDeclarationType::Texture2DMS;
		if (typeStr == "TextureCube") return PropertyDeclarationType::TextureCube;
		if (typeStr == "Matrix3X3" || typeStr == "Matrix3") return PropertyDeclarationType::Matrix3;
		if (typeStr == "Matrix4x4" || typeStr == "Matrix4") return PropertyDeclarationType::Matrix4;

		std::regex rangeRegex(R"(Range\s*\(\s*([+-]?\d*\.?\d+)\s*,\s*([+-]?\d*\.?\d+)\s*\))");
		std::smatch rangeMatch;
		if (std::regex_match(typeStr, rangeMatch, rangeRegex))
		{
			outMin = std::stof(rangeMatch[1].str());
			outMax = std::stof(rangeMatch[2].str());
			return PropertyDeclarationType::Range;
		}

		if (typeStr.find("Enum") == 0) return PropertyDeclarationType::Enum;

		return PropertyDeclarationType::Float;
	}

	std::string ShaderParser::PropertyTypeToGLSL(PropertyDeclarationType type)
	{
		switch (type)
		{
		case PropertyDeclarationType::Bool:		return "uniform bool";
		case PropertyDeclarationType::Color:		return "uniform vec4";
		case PropertyDeclarationType::Color3:	return "uniform vec3";
		case PropertyDeclarationType::Enum:		return "uniform int";
		case PropertyDeclarationType::Float:		return "uniform float";
		case PropertyDeclarationType::Int:			return "uniform int";
		case PropertyDeclarationType::Vector2:		return "uniform vec2";
		case PropertyDeclarationType::Vector3:		return "uniform vec3";
		case PropertyDeclarationType::Vector4:		return "uniform vec4";
		case PropertyDeclarationType::Texture2D:	return "uniform sampler2D";
		case PropertyDeclarationType::Texture2DMS:	return "uniform sampler2DMS";
		case PropertyDeclarationType::TextureCube:	return "uniform samplerCube";
		case PropertyDeclarationType::Range:		return "uniform float";
		case PropertyDeclarationType::Matrix3:		return "uniform mat3";
		case PropertyDeclarationType::Matrix4:		return "uniform mat4";
		default:									return "uniform float";
		}
	}

	int ShaderParser::GetLocationBySemantic(const std::string& semantic, VertexSemantic& outSemantic)
	{
		std::string upper = semantic;
		std::transform(semantic.begin(), semantic.end(), upper.begin(), ::toupper);
		if (upper == "POSITION") { outSemantic = VertexSemantic::Position; return static_cast<int>(outSemantic); }
		if (upper == "NORMAL") { outSemantic = VertexSemantic::Normal; return static_cast<int>(outSemantic); }
		if (upper == "TANGENT") { outSemantic = VertexSemantic::Tangent; return static_cast<int>(outSemantic); }
		if (upper == "BINORMAL") { outSemantic = VertexSemantic::Binormal; return static_cast<int>(outSemantic); }
		if (upper == "TEXCOORD0") { outSemantic = VertexSemantic::TexCoord0; return static_cast<int>(outSemantic); }
		if (upper == "TEXCOORD1") { outSemantic = VertexSemantic::TexCoord1; return static_cast<int>(outSemantic); }
		if (upper == "BONEINDICES") { outSemantic = VertexSemantic::BoneIndices; return static_cast<int>(outSemantic); }
		if (upper == "BONEWEIGHTS") { outSemantic = VertexSemantic::BoneWeights; return static_cast<int>(outSemantic); }
		if (upper == "INSTANCEID") { outSemantic = VertexSemantic::InstanceID; return static_cast<int>(outSemantic); }
		if (upper == "COLOR") { outSemantic = VertexSemantic::Color; return static_cast<int>(outSemantic); }
		if (upper == "INDEX0") { outSemantic = VertexSemantic::Index0; return static_cast<int>(outSemantic); }
		if (upper == "INDEX1") { outSemantic = VertexSemantic::Index1; return static_cast<int>(outSemantic); }
		if (upper == "OTHER0") { outSemantic = VertexSemantic::Other0; return static_cast<int>(outSemantic); }
		if (upper == "OTHER1") { outSemantic = VertexSemantic::Other1; return static_cast<int>(outSemantic); }
		if (upper == "OTHER2") { outSemantic = VertexSemantic::Other2; return static_cast<int>(outSemantic); }
		outSemantic = VertexSemantic::Unknown;
		return -1;
	}

	ParseResult ShaderParser::Parse(const std::string& source)
	{
		ParseResult result;
		result.Success = true;

		std::string cleanCode = StripComments(source);

		if (!ExtractShaderMetadata(cleanCode, result))
		{
			std::cerr << "Error: Failed to parse Shader name." << std::endl;
			result.Success = false;
		}

		if (!ParsePropertiesBlock(cleanCode, result))
		{
			std::cerr << "Error: Failed to parse Properties block." << std::endl;
			result.Success = false;
		}

		ParseRenderCommand(cleanCode, result);

		if (!ParseSubShader(cleanCode, result))
		{
			std::cerr << "Error: Failed to parse SubShader block." << std::endl;
			result.Success = false;
		}

		std::filesystem::path rootPath = std::filesystem::path(s_IncludeRoot);
		for (auto& pass : result.Passes)
		{
			std::set<std::filesystem::path> history;
			pass.ProcessedGLSL = ResolveIncludes(pass.RawGLSL, rootPath, history);
			ProcessAttributes(pass);
			InjectHeader(pass, result);
			SplitShader(pass);
		}

		return result;
	}

#pragma region Raw Processing

	static std::string Trim(const std::string& s)
	{
		auto start = s.find_first_not_of(" \t\n\r");
		auto end = s.find_last_not_of(" \t\n\r");
		return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
	}

	static std::string ExtractBlock(const std::string& source, const std::string& key, size_t offset, size_t& outBlockEnd)
	{
		size_t keyPos = source.find(key, offset);
		if (keyPos == std::string::npos) return "";

		size_t openBrace = source.find('{', keyPos);
		if (openBrace == std::string::npos) return "";

		int braceCount = 1;
		for (size_t i = openBrace + 1; i < source.size(); ++i)
		{
			if (source[i] == '{') braceCount++;
			else if (source[i] == '}') braceCount--;

			if (braceCount == 0)
			{
				outBlockEnd = i;
				return source.substr(openBrace + 1, i - openBrace - 1);
			}
		}
		return "";
	}

	std::string ShaderParser::StripComments(const std::string& source)
	{
		std::string result;
		result.reserve(source.size());
		bool inSingleLine = false;
		bool inMultiLine = false;
		bool inString = false;

		for (size_t i = 0; i < source.size(); ++i)
		{
			char c = source[i];
			char next = (i + 1 < source.size()) ? source[i + 1] : '\0';

			if (!inSingleLine && !inMultiLine)
			{
				if (c == '"' && (i == 0 || source[i - 1] != '\\'))
					inString = !inString;
			}

			if (inString)
			{
				result += c;
				continue;
			}

			if (!inMultiLine && !inSingleLine && c == '/' && next == '/')
			{
				inSingleLine = true;
				i++; continue;
			}
			if (inSingleLine && c == '\n')
			{
				inSingleLine = false;
				result += c; continue;
			}

			if (!inSingleLine && !inMultiLine && c == '/' && next == '*')
			{
				inMultiLine = true;
				i++; continue;
			}
			if (inMultiLine && c == '*' && next == '/')
			{
				inMultiLine = false;
				i++; continue;
			}

			if (!inSingleLine && !inMultiLine)
				result += c;
			else if (inMultiLine && c == '\n')
				result += c;
		}
		return result;
	}

	bool ShaderParser::ExtractShaderMetadata(const std::string& source, ParseResult& outResult)
	{
		std::regex shaderRegex(R"prism(Shader\s+"([^"]+)")prism");
		std::smatch match;
		if (std::regex_search(source, match, shaderRegex))
		{
			outResult.ShaderName = match[1].str();
			return true;
		}
		return false;
	}

	bool ShaderParser::ParseRenderCommand(const std::string& source, ParseResult& result)
	{
		size_t cmdEnd = 0;
		result.RenderCommand = ExtractBlock(source, "RenderCommand", 0, cmdEnd);
		return !result.RenderCommand.empty();
	}

	bool ShaderParser::ParsePropertiesBlock(const std::string& source, ParseResult& outResult)
	{
		size_t propStart = source.find("Properties");
		if (propStart == std::string::npos) return true;

		size_t openBrace = source.find('{', propStart);
		if (openBrace == std::string::npos) return false;

		int braceCount = 1;
		size_t closeBrace = std::string::npos;
		for (size_t i = openBrace + 1; i < source.size(); ++i)
		{
			if (source[i] == '{') braceCount++;
			else if (source[i] == '}') braceCount--;
			if (braceCount == 0) { closeBrace = i; break; }
		}
		if (closeBrace == std::string::npos) return false;

		std::string blockContent = source.substr(openBrace + 1, closeBrace - openBrace - 1);

		std::regex propLineRegex(
			R"prism((\w+)\s*\(\s*"([^"]*)"\s*,\s*([\w\s\(\),\.-]+)\)\s*=\s*((?:"[^"]*"(?:\s*\{\s*\})?|\{\s*\}|\([^\)]*\)|[^\s;]+)))prism"
		);
		auto words_begin = std::sregex_iterator(blockContent.begin(), blockContent.end(), propLineRegex);
		auto words_end = std::sregex_iterator();

		for (std::sregex_iterator i = words_begin; i != words_end; ++i)
		{
			std::smatch match = *i;
			PropertyDescriptor desc;
			desc.Name = match[1].str();
			desc.DisplayName = match[2].str();
			desc.DefaultValue = match[4].str();

			std::string typePart = Trim(match[3].str());
			desc.Type = StringToPropertyType(typePart, desc.Min, desc.Max);

			if (desc.Type == PropertyDeclarationType::Enum)
			{
				size_t open = typePart.find('(');
				size_t close = typePart.find(')');
				if (open != std::string::npos && close != std::string::npos && close > open)
				{
					std::string optionsStr = typePart.substr(open + 1, close - open - 1);
					std::stringstream ss(optionsStr);
					std::string option;
					while (std::getline(ss, option, ','))
						desc.EnumOptions.push_back(Trim(option));
				}
			}

			outResult.Properties.push_back(desc);
		}
		return true;
	}

	bool ShaderParser::ParseSubShader(const std::string& source, ParseResult& outResult)
	{
		size_t subShaderEnd = 0;
		std::string subShaderContent = ExtractBlock(source, "SubShader", 0, subShaderEnd);
		if (subShaderContent.empty()) return false;

		size_t searchPos = 0;
		while (true)
		{
			size_t passPos = subShaderContent.find("Pass", searchPos);
			if (passPos == std::string::npos) break;

			size_t passEnd = 0;
			std::string passContent = ExtractBlock(subShaderContent, "Pass", passPos, passEnd);

			if (!passContent.empty())
			{
				PassDescriptor pass;
				ParsePassInternal(passContent, pass);
				outResult.Passes.push_back(pass);
				searchPos = passEnd + 1;
			}
			else
			{
				break;
			}
		}
		return !outResult.Passes.empty();
	}

	void ShaderParser::ParsePassInternal(const std::string& passContent, PassDescriptor& outPass)
	{
		std::regex nameRegex(R"prism(Name\s+"([^"]+)")prism");
		std::smatch nameMatch;
		if (std::regex_search(passContent, nameMatch, nameRegex))
			outPass.Name = nameMatch[1].str();

		size_t tagsEnd = 0;
		std::string tagsBody = ExtractBlock(passContent, "Tags", 0, tagsEnd);
		if (!tagsBody.empty())
		{
			std::regex tagKVRegex(R"prism("([^"]+)"\s*=\s*"([^"]+)")prism");
			auto tags_begin = std::sregex_iterator(tagsBody.begin(), tagsBody.end(), tagKVRegex);
			auto tags_end = std::sregex_iterator();
			for (std::sregex_iterator i = tags_begin; i != tags_end; ++i)
				outPass.Tags[(*i)[1].str()] = (*i)[2].str();
		}

		size_t glslEnd = 0;
		outPass.RawGLSL = ExtractBlock(passContent, "GLSL", 0, glslEnd);
	}

	std::string ShaderParser::ResolveIncludes(const std::string& source, const std::filesystem::path& root, std::set<std::filesystem::path>& history)
	{
		static std::regex includeRegex(R"prism(#include\s+"([^"]+)")prism");
		std::string processed;
		std::istringstream stream(source);
		std::string line;

		while (std::getline(stream, line))
		{
			std::smatch match;
			if (std::regex_search(line, match, includeRegex))
			{
				std::filesystem::path fileName = match[1].str();
				std::filesystem::path fullPath = root / fileName;

				if (history.find(fullPath) != history.end()) continue;

				std::ifstream file(fullPath);
				if (!file.is_open())
				{
					processed += "// Error: Include not found: " + fileName.string() + "\n";
					continue;
				}

				std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();

				history.insert(fullPath);
				processed += ResolveIncludes(content, root, history) + "\n";
			}
			else
			{
				processed += line + "\n";
			}
		}
		return processed;
	}

#pragma endregion

#pragma region GLSL Processing

	void ShaderParser::ProcessAttributes(PassDescriptor& pass)
	{
		static std::regex attrRegex(R"prism(attribute\s+([\w\d_]+)\s+([\w\d_]+)\s*:\s*([\w\d_]+)\s*;)prism");
		std::string& code = pass.ProcessedGLSL;
		auto it = std::sregex_iterator(code.begin(), code.end(), attrRegex);
		auto end = std::sregex_iterator();

		std::string newCode = code;
		size_t offset = 0;

		for (; it != end; ++it)
		{
			std::smatch match = *it;
			VertexAttributeDescriptor attr;
			attr.Type = match[1].str();
			attr.Name = match[2].str();
			attr.SemanticStr = match[3].str();
			attr.Location = GetLocationBySemantic(attr.SemanticStr, attr.Semantic);
			pass.Attributes.push_back(attr);

			std::string replacement = "layout(location = " + std::to_string(attr.Location) + ") in " + attr.Type + " " + attr.Name + ";";

			size_t startPos = match.position() + offset;
			newCode.replace(startPos, match.length(), replacement);
			offset += (replacement.length() - match.length());
		}
		code = newCode;
	}

	void ShaderParser::InjectHeader(PassDescriptor& pass, const ParseResult& result)
	{
		std::stringstream header;

		for (const auto& prop : result.Properties)
			header << PropertyTypeToGLSL(prop.Type) << " " << prop.Name << ";\n";
		header << "\n";

		header << "#ifdef PRISM_VERTEX_SHADER\n";
		header << "    #define VARYING out\n";
		header << "#else\n";
		header << "    #define VARYING in\n";
		header << "    layout(location = 0) out vec4 FragColor;\n";
		header << "#endif\n\n";

		pass.ProcessedGLSL.insert(0, header.str());
	}

	void ShaderParser::SplitShader(PassDescriptor& pass)
	{
		std::string baseCode = pass.ProcessedGLSL;

		std::string vsCode = s_VersionHeader + "#define PRISM_VERTEX_SHADER\n" + baseCode;
		RemoveFunction(vsCode, "frag");
		pass.VertexShaderCode = vsCode;

		std::string fsCode = s_VersionHeader + "#define PRISM_FRAGMENT_SHADER\n" + baseCode;
		RemoveFunction(fsCode, "main");

		static std::regex attrCleanupRegex(R"prism(layout\s*\(\s*location\s*=\s*\d+\s*\)\s*in\s+[^;]+;)prism");
		fsCode = std::regex_replace(fsCode, attrCleanupRegex, "");

		static std::regex fragSignatureRegex(R"prism(void\s+frag\s*\(\s*\))prism");
		if (std::regex_search(fsCode, fragSignatureRegex))
			fsCode = std::regex_replace(fsCode, fragSignatureRegex, "void main()");

		pass.FragmentShaderCode = fsCode;

		pass.VertexShaderCode = StripComments(pass.VertexShaderCode);
		pass.FragmentShaderCode = StripComments(pass.FragmentShaderCode);
		FormatCodeInPlace(pass.VertexShaderCode);
		FormatCodeInPlace(pass.FragmentShaderCode);
	}

	void ShaderParser::RemoveFunction(std::string& code, const std::string& funcName)
	{
		std::regex funcHeadRegex("void\\s+" + funcName + "\\s*\\(\\s*\\)\\s*\\{");
		std::smatch match;

		if (std::regex_search(code, match, funcHeadRegex))
		{
			size_t startPos = match.position();
			size_t openBracePos = startPos + match.length() - 1;

			int braceCount = 1;
			size_t endPos = std::string::npos;
			for (size_t i = openBracePos + 1; i < code.size(); ++i)
			{
				if (code[i] == '{') braceCount++;
				else if (code[i] == '}') braceCount--;
				if (braceCount == 0) { endPos = i; break; }
			}

			if (endPos != std::string::npos)
				code.erase(startPos, endPos - startPos + 1);
		}
	}

	void ShaderParser::FormatCodeInPlace(std::string& code)
	{
		std::istringstream stream(code);
		std::string line;
		std::string result;
		int indent = 0;

		while (std::getline(stream, line))
		{
			line = Trim(line);
			if (line.empty()) continue;

			if (line.find('}') != std::string::npos)
				for (char c : line) if (c == '}') indent--;

			for (int i = 0; i < (std::max)(0, indent); ++i) result += "    ";
			result += line + "\n";

			if (line.find('{') != std::string::npos)
				for (char c : line) if (c == '{') indent++;
		}
		code = result;
	}

#pragma endregion
}
