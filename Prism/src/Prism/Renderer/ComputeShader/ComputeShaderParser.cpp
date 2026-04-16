#include "prpch.h"
#include "ComputeShaderParserData.h"
#include "ComputeShaderParser.h"

#include <regex>

namespace Prism
{
	using namespace StrParse;
	static std::string StripComments(const std::string& source)
	{
		std::string result;
		result.reserve(source.size());
		bool inSingleLine = false, inMultiLine = false, inString = false;

		for (size_t i = 0; i < source.size(); ++i)
		{
			char c = source[i];
			char next = (i + 1 < source.size()) ? source[i + 1] : '\0';

			if (inString)
			{
				if (c == '"' && (i == 0 || source[i - 1] != '\\')) inString = false;
				result += c;
				continue;
			}
			if (c == '"' && (i == 0 || source[i - 1] != '\\')) { inString = true; result += c; continue; }
			if (!inMultiLine && !inSingleLine && c == '/' && next == '/') { inSingleLine = true; i++; continue; }
			if (inSingleLine && c == '\n') { inSingleLine = false; result += c; continue; }
			if (!inSingleLine && !inMultiLine && c == '/' && next == '*') { inMultiLine = true; i++; continue; }
			if (inMultiLine && c == '*' && next == '/') { inMultiLine = false; i++; continue; }
			if (!inSingleLine && !inMultiLine) result += c;
			else if (inMultiLine && c == '\n') result += c;
		}
		return result;
	}

	static std::tuple<bool, std::string, std::string, std::string> ParseBufferDeclaration(const std::string& line)
	{
		static const std::regex re(R"(\b(\w+)\s*<\s*([^>]+)\s*>\s+(\w+)\s*;)");
		std::smatch m;
		if (std::regex_search(line, m, re))
			return { true, m[1].str(), m[2].str(), m[3].str() };
		return { false, "", "", "" };
	}

	static std::tuple<bool, int, int, int> ParseNumThreads(const std::string& line)
	{
		static const std::regex re(R"(\[\s*numthreads\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)\s*\])");
		std::smatch m;
		if (std::regex_search(line, m, re))
		{
			try { return { true, std::stoi(m[1]), std::stoi(m[2]), std::stoi(m[3]) }; }
			catch (...) {}
		}
		return { false, 1, 1, 1 };
	}

	static void ProcessPragma(Tokens& tokens, Lines::iterator& it, ComputeShaderResult& data)
	{
		tokens.pop();
		if (!tokens.empty() && tokens.front() == "kernel")
		{
			tokens.pop();
			if (!tokens.empty())
			{
				KernelInfo info;
				info.name = tokens.front();
				info.line = it;
				data.kernels.push_back(info);
			}
			*it = "";
		}
	}

	static void ProcessBuffer(const std::string& type, const std::string& param, const std::string& name,
		ComputeShaderResult& data, uint32_t& bindingIndex, std::string& line)
	{
		std::string layout;
		ComputeShaderResourceType resType = ComputeShaderResourceType::None;

		if (type == "RBuffer")
		{
			resType = ComputeShaderResourceType::RBuffer;
			layout = "layout(std430, binding = " + std::to_string(bindingIndex) + ") readonly buffer " + name + "Buffer {\n    " + param + " " + name + "[];\n};";
		}
		else if (type == "WBuffer")
		{
			resType = ComputeShaderResourceType::WBuffer;
			layout = "layout(std430, binding = " + std::to_string(bindingIndex) + ") writeonly buffer " + name + "Buffer {\n    " + param + " " + name + "[];\n};";
		}
		else if (type == "RWBuffer")
		{
			resType = ComputeShaderResourceType::RWBuffer;
			layout = "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer " + name + "Buffer {\n    " + param + " " + name + "[];\n};";
		}
		else if (type == "RWImage2D")
		{
			resType = ComputeShaderResourceType::RWImage2D;
			layout = "layout(" + param + ", binding = " + std::to_string(bindingIndex) + ") uniform image2D " + name + ";";
		}

		if (!layout.empty())
		{
			ComputeShaderResource res{ resType, name, param, bindingIndex };
			data.resources.push_back(res);
			line = layout;
			bindingIndex++;
		}
	}
	static void ProcessKernel(Tokens& tokens, ComputeShaderResult& data, Lines::iterator& it)
	{
		tokens.pop();
		for (auto& kernel : data.kernels)
		{
			if (kernel.name == tokens.front() || kernel.name + "()" == tokens.front())
			{
				auto [hasThread, x, y, z] = ParseNumThreads(*(it - 1));
				if (hasThread)
				{
					kernel.numThreads[0] = x;
					kernel.numThreads[1] = y;
					kernel.numThreads[2] = z;
					*(it - 1) = "";
					*it = "void main()";
				}
				break;
			}
		}
	}
	static void ProcessLines(Lines& lines, ComputeShaderResult& result)
	{
		uint32_t bindingIndex = 0;
		for (auto it = lines.begin(); it != lines.end(); ++it)
		{
			std::string line = Trim(*it);
			if (line.empty()) continue;

			Tokens tokens;
			std::stringstream ss(line);
			std::string token;
			while (ss >> token) tokens.push(token);

			if (tokens.empty()) continue;

			if (tokens.front() == "#pragma")
				ProcessPragma(tokens, it, result);
			else if (tokens.front() == "void")
				ProcessKernel(tokens, result, it);

			auto [isBuffer, bufType, bufParam, bufName] = ParseBufferDeclaration(*it);
			if (isBuffer)
				ProcessBuffer(bufType, bufParam, bufName, result, bindingIndex, *it);
		}
	}
	static void GenerateKernelSources(Lines& lines, ComputeShaderResult& result)
	{
		for (auto& kernel : result.kernels)
		{
			std::string header = "#version 450 core\nlayout(local_size_x = " +
				std::to_string(kernel.numThreads[0]) + ", local_size_y = " +
				std::to_string(kernel.numThreads[1]) + ", local_size_z = " +
				std::to_string(kernel.numThreads[2]) + ") in;\n";

			Lines kernelLines = lines;
			for (auto& l : kernelLines)
			{
				if (Trim(l) == "void main()")
					l = "void main()";
			}

			kernelLines.insert(kernelLines.begin(), header);
			MergeLines(kernelLines, kernel.source);
		}
	}
	ComputeShaderResult ComputeShaderParser::Parse(const std::string& shaderCode)
	{
		ComputeShaderResult result;
		result.source = StripComments(shaderCode);
		Lines lines;
		SplitLines(result.source, lines);
		ProcessLines(lines, result);
		GenerateKernelSources(lines, result);

		return result;
	}

}