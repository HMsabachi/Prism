#include "PrismShaderParser.h"
#include <sstream>
#include <iostream>
#include <algorithm>

namespace Prism
{
    std::string PrismShaderParser::s_VersionHeader = "#version 450 core\n";
    std::string PrismShaderParser::s_IncludeRoot = "Assets/Shaders/";
    std::string PrismShaderParser::s_FileHeader = R"PRISM(
// 暂无
)PRISM";
    PropertyType PrismShaderParser::StringToPropertyType(const std::string& typeStr, float& outMin, float& outMax)
    {
        if (typeStr == "Color") return PropertyType::Color;
        if (typeStr == "Float") return PropertyType::Float;
        if (typeStr == "Int")   return PropertyType::Int;
        if (typeStr == "Vector2") return PropertyType::Vector2;
        if (typeStr == "Vector3") return PropertyType::Vector3;
        if (typeStr == "Vector4") return PropertyType::Vector4;
        if (typeStr == "Texture2D") return PropertyType::Texture2D;
        std::regex rangeRegex(R"(Range\s*\(\s*([+-]?\d*\.?\d+)\s*,\s*([+-]?\d*\.?\d+)\s*\))");
        std::smatch rangeMatch;
        if (std::regex_match(typeStr, rangeMatch, rangeRegex)) {
            outMin = std::stof(rangeMatch[1].str());
            outMax = std::stof(rangeMatch[2].str());
            return PropertyType::Range;
        }

        return PropertyType::Float;
    }
    std::string PrismShaderParser::PropertyTypeToString(PropertyType type)
    {
        switch (type)
        {
        case PropertyType::Color: return "uniform vec4"; break;
        case PropertyType::Float: return "uniform float"; break;
        case PropertyType::Int: return "uniform int"; break;
        case PropertyType::Vector2: return "uniform vec2"; break;
        case PropertyType::Vector3: return "uniform vec3"; break;
        case PropertyType::Vector4: return "uniform vec4"; break;
        case PropertyType::Texture2D: return "uniform sampler2D"; break;
        case PropertyType::Range: return "uniform float"; break;
        }
    }
    ParseResult PrismShaderParser::Parse(const std::string& source)
    {
        ParseResult result;
        // 1. 去除注释
        std::string cleanCode = StripComments(source);
        // 2. 提取基础信息
        if (!ExtractShaderMetadata(cleanCode, result)) {
            std::cerr << "Error: Failed to parse Shader name." << std::endl;
        }
        // 3. 提取属性
        if (!ParsePropertiesBlock(cleanCode, result)) {
            std::cerr << "Error: Failed to parse Properties block." << std::endl;
        }
        // 执行 SubShader 和 Pass 的解析
        if (!ParseSubShader(cleanCode, result)) {
            std::cerr << "Error: Failed to parse SubShader block." << std::endl;
        }
        // 4. 处理 #include 指令
        std::filesystem::path rootPath = std::filesystem::path(s_IncludeRoot);
        for (auto& pass : result.Passes) {
            std::set<std::filesystem::path> history;
            // 1. 处理 #include 指令
            pass.ProcessedGLSL = ResolveIncludes(pass.RawGLSL, rootPath, history);
            // 2. 解析并替换 Attribute
            ProcessAttributes(pass);
            // 3. 注入 Header (Uniforms, Blocks, Macros)
            InjectHeader(pass, result);
            // 4. 分割 GLSL 代码块
            SplitShader(pass);
        }

        return result;
    }
    // 辅助函数：去除首尾空格
    static std::string Trim(const std::string& s);
    // 辅助函数：提取块
    static std::string ExtractBlock(const std::string& source, const std::string& key, size_t offset, size_t& outBlockEnd);
#pragma region 初步处理
    std::string PrismShaderParser::StripComments(const std::string& source)
    {
        std::string result;
        result.reserve(source.size());
        bool inSingleLineComment = false;
        bool inMultiLineComment = false;
        bool inString = false;

        for (size_t i = 0; i < source.size(); ++i) {
            char c = source[i];
            char next = (i + 1 < source.size()) ? source[i + 1] : '\0';

            if (!inSingleLineComment && !inMultiLineComment) {
                if (c == '"' && (i == 0 || source[i - 1] != '\\')) {
                    inString = !inString;
                }
            }

            if (inString) {
                result += c;
                continue;
            }

            if (!inMultiLineComment && !inSingleLineComment && c == '/' && next == '/') {
                inSingleLineComment = true;
                i++; continue;
            }
            if (inSingleLineComment && c == '\n') {
                inSingleLineComment = false;
                result += c; continue;
            }

            if (!inSingleLineComment && !inMultiLineComment && c == '/' && next == '*') {
                inMultiLineComment = true;
                i++; continue;
            }
            if (inMultiLineComment && c == '*' && next == '/') {
                inMultiLineComment = false;
                i++; continue;
            }

            if (!inSingleLineComment && !inMultiLineComment) {
                result += c;
            }
            else if (inMultiLineComment && c == '\n') {
                result += c;
            }
        }
        return result;
    }

    bool PrismShaderParser::ExtractShaderMetadata(const std::string& source, ParseResult& outResult)
    {
        // 修复：增加了对换行和多余空格的宽容度
        std::regex shaderRegex(R"prism(Shader\s+"([^"]+)")prism");
        std::smatch match;
        if (std::regex_search(source, match, shaderRegex)) {
            outResult.ShaderName = match[1].str();
            return true;
        }
        return false;
    }

    bool PrismShaderParser::ParsePropertiesBlock(const std::string& source, ParseResult& outResult)
    {
        size_t propStart = source.find("Properties");
        if (propStart == std::string::npos) return true;

        size_t openBrace = source.find('{', propStart);
        if (openBrace == std::string::npos) return false;

        int braceCount = 1;
        size_t closeBrace = std::string::npos;
        for (size_t i = openBrace + 1; i < source.size(); ++i) {
            if (source[i] == '{') braceCount++;
            else if (source[i] == '}') braceCount--;
            if (braceCount == 0) { closeBrace = i; break; }
        }

        if (closeBrace == std::string::npos) return false;

        std::string blockContent = source.substr(openBrace + 1, closeBrace - openBrace - 1);

        //重点修复正则：
           //Group 1: 变量名 (\w+)
           //Group 2: 显示名 ([^"]*)
           //Group 3: 类型名 ([\w\s\(\),\.-]+) -> 支持 Range(0, 1)
           //Group 4: 默认值 ("[^"]*"|\([^\)]*\)|[^\s{}]+) -> 支持 "white" {}, (1,1,1,1), 或 0.5

        std::regex propLineRegex(R"prism((\w+)\s*\(\s*"([^"]*)"\s*,\s*([\w\s\(\),\.-]+)\)\s*=\s*("[^"]*"(?:\s*\{?\s*\}?)?|\([^\)]*\)|[^\s{}]+))prism");

        auto words_begin = std::sregex_iterator(blockContent.begin(), blockContent.end(), propLineRegex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            PropertyDescriptor desc;
            desc.Name = match[1].str();
            desc.DisplayName = match[2].str();
            desc.DefaultValue = match[4].str();

            std::string typePart = Trim(match[3].str());
            desc.Type = StringToPropertyType(typePart, desc.Min, desc.Max);

            outResult.Properties.push_back(desc);
        }
        return true;
    }

    bool PrismShaderParser::ParseSubShader(const std::string& source, ParseResult& outResult) {
        size_t subShaderEnd = 0;
        // 1. 提取 SubShader 块内容
        std::string subShaderContent = ExtractBlock(source, "SubShader", 0, subShaderEnd);
        if (subShaderContent.empty()) return false;

        // 2. 在 SubShader 内容中循环查找所有 Pass
        size_t searchPos = 0;
        while (true) {
            size_t passPos = subShaderContent.find("Pass", searchPos);
            if (passPos == std::string::npos) break;

            size_t passEnd = 0;
            std::string passContent = ExtractBlock(subShaderContent, "Pass", passPos, passEnd);

            if (!passContent.empty()) {
                PassDescriptor pass;
                ParsePassInternal(passContent, pass);
                outResult.Passes.push_back(pass);
                searchPos = passEnd + 1; // 从当前 Pass 结束后继续寻找下一个
            }
            else {
                break;
            }
        }
        return !outResult.Passes.empty();
    }

    void PrismShaderParser::ParsePassInternal(const std::string& passContent, PassDescriptor& outPass) {
        // 1. 提取 Pass 名称: Name "ForwardBase"
        std::regex nameRegex(R"prism(Name\s+"([^"]+)")prism");
        std::smatch nameMatch;
        if (std::regex_search(passContent, nameMatch, nameRegex)) {
            outPass.Name = nameMatch[1].str();
        }

        // 2. 提取并解析 Tags: Tags { "Queue" = "Geometry" ... }
        size_t tagsEnd = 0;
        std::string tagsBody = ExtractBlock(passContent, "Tags", 0, tagsEnd);
        if (!tagsBody.empty()) {
            std::regex tagKVRegex(R"prism("([^"]+)"\s*=\s*"([^"]+)")prism");
            auto tags_begin = std::sregex_iterator(tagsBody.begin(), tagsBody.end(), tagKVRegex);
            auto tags_end = std::sregex_iterator();
            for (std::sregex_iterator i = tags_begin; i != tags_end; ++i) {
                outPass.Tags[(*i)[1].str()] = (*i)[2].str();
            }
        }

        // 3. 提取 GLSL 原始代码块
        size_t glslEnd = 0;
        outPass.RawGLSL = ExtractBlock(passContent, "GLSL", 0, glslEnd);
    }

    std::string PrismShaderParser::ResolveIncludes(const std::string& source,
        const std::filesystem::path& root,
        std::set<std::filesystem::path>& history) {
        // 匹配 #include "filename"
        static std::regex includeRegex(R"prism(#include\s+"([^"]+)")prism");
        std::string processed;
        std::istringstream stream(source);
        std::string line;

        while (std::getline(stream, line)) {
            std::smatch match;
            if (std::regex_search(line, match, includeRegex)) {
                std::filesystem::path fileName = match[1].str();
                std::filesystem::path fullPath = root / fileName;

                // 循环引用检查
                if (history.find(fullPath) != history.end()) continue;

                std::ifstream file(fullPath);
                if (!file.is_open()) {
                    // TODO: 这里有ERROR处理
                    processed += "// Error: Include not found: " + fileName.string() + "\n";
                    continue;
                }

                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                file.close();

                history.insert(fullPath);
                processed += ResolveIncludes(content, root, history) + "\n";
            }
            else {
                processed += line + "\n";
            }
        }
        return processed;
    }
#pragma endregion

#pragma region 分离翻译为GLSL
    void PrismShaderParser::ProcessAttributes(PassDescriptor& pass) {
        // 正则匹配: attribute 类型 变量名 : 语义;
        // Group 1: 类型 (vec3), Group 2: 变量名 (aPos), Group 3: 语义 (POSITION)
        static std::regex attrRegex(R"prism(attribute\s+([\w\d_]+)\s+([\w\d_]+)\s*:\s*([\w\d_]+)\s*;)prism");

        std::string& code = pass.ProcessedGLSL;
        auto it = std::sregex_iterator(code.begin(), code.end(), attrRegex);
        auto end = std::sregex_iterator();

        std::string newCode = code;
        size_t offset = 0;

        for (; it != end; ++it) {
            std::smatch match = *it;
            VertexAttributeDescriptor attr;
            attr.Type = match[1].str();
            attr.Name = match[2].str();
            attr.SemanticStr = match[3].str();
            attr.Location = GetLocationBySemantic(attr.SemanticStr, attr.Semantic);

            pass.Attributes.push_back(attr);

            // 生成标准 GLSL 替换字符串
            std::string replacement = "layout(location = " + std::to_string(attr.Location) + ") in " + attr.Type + " " + attr.Name + ";";

            // 在新字符串中替换（注意处理由于长度变化导致的位移）
            size_t startPos = match.position() + offset;
            newCode.replace(startPos, match.length(), replacement);
            offset += (replacement.length() - match.length());
        }
        code = newCode;
    }
    void PrismShaderParser::InjectHeader(PassDescriptor& pass, const ParseResult& result) {
        std::stringstream header;

        // 1. 注入版本 (工业级通常由引擎控制版本，方便跨平台)
        header << s_FileHeader;

        // 3. 注入材质 Properties 转换的 Uniforms
        header << "// ---- Material Properties ----\n";
        for (const auto& prop : result.Properties) {
            header << PropertyTypeToString(prop.Type) << " " << prop.Name << ";\n";
        }
        header << "\n";

        // 4. 注入 VARYING 宏处理 (为下一步代码拆分做准备)
        header << "#ifdef PRISM_VERTEX_SHADER\n";
        header << "    #define VARYING out\n";
        header << "#else\n";
        header << "    #define VARYING in\n";
        header << "    layout(location = 0) out vec4 FragColor;\n";
        header << "#endif\n\n";

        pass.ProcessedGLSL.insert(0, header.str());
    }
    void PrismShaderParser::SplitShader(PassDescriptor& pass) {
        std::string baseCode = pass.ProcessedGLSL;

        // --- 处理顶点着色器 (VS) ---
        std::string vsCode = s_VersionHeader + "#define PRISM_VERTEX_SHADER\n" + baseCode;
        RemoveFunction(vsCode, "frag");
        // VS 需要保留 layout(location...) in，所以不需要额外处理
        pass.VertexShaderCode = vsCode;

        // --- 处理片元着色器 (FS) ---
        std::string fsCode = s_VersionHeader + "#define PRISM_FRAGMENT_SHADER\n" + baseCode;

        // 1. 删掉原来的 main (顶点函数的入口)
        RemoveFunction(fsCode, "main");

        // 2. 【新增步骤】删掉顶点特有的输入属性 (Attributes)
        // 匹配 layout(location = n) in 类型 变量名;
        static std::regex attrCleanupRegex(R"prism(layout\s*\(\s*location\s*=\s*\d+\s*\)\s*in\s+[^;]+;)prism");
        fsCode = std::regex_replace(fsCode, attrCleanupRegex, "");

        // 3. 将 frag 改名为 main
        static std::regex fragSignatureRegex(R"prism(void\s+frag\s*\(\s*\))prism");
        if (std::regex_search(fsCode, fragSignatureRegex)) {
            fsCode = std::regex_replace(fsCode, fragSignatureRegex, "void main()");
        }

        pass.FragmentShaderCode = fsCode;
        // 4. 最后进行美化
        FormatCodeInPlace(pass.VertexShaderCode);
        FormatCodeInPlace(pass.FragmentShaderCode);
    }
    void PrismShaderParser::RemoveFunction(std::string& code, const std::string& funcName) {
        // 匹配 void funcName() 及其后面的左大括号
        // ([^{]*) 捕获函数名和括号之间可能存在的空格或换行
        std::regex funcHeadRegex("void\\s+" + funcName + "\\s*\\(\\s*\\)\\s*\\{");
        std::smatch match;

        if (std::regex_search(code, match, funcHeadRegex)) {
            size_t startPos = match.position();
            size_t openBracePos = startPos + match.length() - 1; // 最后一个字符是 '{'

            // 寻找匹配的右大括号
            int braceCount = 1;
            size_t endPos = std::string::npos;
            for (size_t i = openBracePos + 1; i < code.size(); ++i) {
                if (code[i] == '{') braceCount++;
                else if (code[i] == '}') braceCount--;

                if (braceCount == 0) {
                    endPos = i;
                    break;
                }
            }

            if (endPos != std::string::npos) {
                // 删除从 void 开始到 结束大括号 的全部内容
                code.erase(startPos, endPos - startPos + 1);
            }
        }
    }
#pragma endregion

#pragma region 后处理
    std::string PrismShaderParser::FormatGLSL(const std::string& code) {
        std::stringstream src(code);
        std::stringstream dst;
        std::string line;
        int indentLevel = 0;

        auto getIndent = [](int level) {
            return std::string(level * 4, ' '); // 4个空格缩进
            };

        bool lastWasEmpty = false;

        while (std::getline(src, line)) {
            // 1. 去除首尾空格
            line = Trim(line);

            // 2. 处理空行：不允许连续两个以上的空行
            if (line.empty()) {
                if (!lastWasEmpty) {
                    dst << "\n";
                    lastWasEmpty = true;
                }
                continue;
            }
            lastWasEmpty = false;

            // 3. 缩进处理逻辑
            // 如果当前行以 '}' 开头，先减少缩进
            if (!line.empty() && line[0] == '}') {
                indentLevel = std::max(0, indentLevel - 1);
            }

            // 写入当前行缩进 + 内容
            dst << getIndent(indentLevel) << line << "\n";

            // 统计行内的大括号，决定下一行的缩进
            // 注意：这里简单处理了，不考虑字符串里的括号
            for (char c : line) {
                if (c == '{') indentLevel++;
                else if (c == '}') {
                    // 如果 '}' 不在行首（比如 };），也需要减缩进
                    // 但如果行首已经减过了，这里要小心逻辑
                }
            }

            // 更严谨的缩进增量：
            // 计算这一行净增的括号数（排除已经在行首处理过的 '}'）
            size_t opens = std::count(line.begin(), line.end(), '{');
            size_t closes = std::count(line.begin(), line.end(), '}');

            // 逻辑修正：
            // 如果行首是 '}'，上面的代码已经减过 1 了，所以这里计算增量时要跳过行首的 '}'
            int netChange = (int)opens - (int)closes;
            if (!line.empty() && line[0] == '}') {
                // 如果行首是 '}'，netChange 里包含的那个 -1 已经生效了，只需处理剩下的
                indentLevel = indentLevel + netChange + 1;
            }
            else {
                // 如果行首不是 '}'，indentLevel 是旧的，现在更新它
                // 注意：我们已经写完了当前行，所以 indentLevel 的变化影响下一行
                // 但如果 line 是 "void main() {"，我们需要在此之后增加 indent
                // 实际上上面的 dst << 已经写完了，所以这里直接更新即可
                // 之前的 if(line[0] == '}') 已经处理了当前行缩进
                // 这里的更新是为了下一行
                indentLevel = std::max(0, indentLevel + netChange);
            }
        }

        return dst.str();
    }
    void PrismShaderParser::FormatCodeInPlace(std::string& code) {
        std::istringstream stream(code);
        std::string line;
        std::string result;
        int indent = 0;

        while (std::getline(stream, line)) {
            line = Trim(line);
            if (line.empty()) continue; // 彻底移除多余空行

            // 遇到右括号，当前行缩进就要减
            if (line.find('}') != std::string::npos) {
                // 统计这一行有几个 } 就减几次，简单粗暴但有效
                for (char c : line) if (c == '}') indent--;
            }

            // 补上缩进
            for (int i = 0; i < std::max(0, indent); ++i) result += "    ";
            result += line + "\n";

            // 遇到左括号，后续行缩进加
            if (line.find('{') != std::string::npos) {
                for (char c : line) if (c == '{') indent++;
            }
        }
        code = result;
    }
#pragma endregion

    // 辅助函数：去除首尾空格
    static std::string Trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\n\r");
        auto end = s.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }
    // 辅助函数：提取块
    static std::string ExtractBlock(const std::string& source, const std::string& key, size_t offset, size_t& outBlockEnd) {
        size_t keyPos = source.find(key, offset);
        if (keyPos == std::string::npos) return "";

        size_t openBrace = source.find('{', keyPos);
        if (openBrace == std::string::npos) return "";

        int braceCount = 1;
        for (size_t i = openBrace + 1; i < source.size(); ++i) {
            if (source[i] == '{') braceCount++;
            else if (source[i] == '}') braceCount--;

            if (braceCount == 0) {
                outBlockEnd = i;
                return source.substr(openBrace + 1, i - openBrace - 1);
            }
        }
        return "";
    }
}