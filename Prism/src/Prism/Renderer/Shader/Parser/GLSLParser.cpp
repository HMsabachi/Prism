#include "prpch.h"
#include "GLSLParser.h"
#include <regex>
#include <sstream>

namespace Prism
{

GLSLParser::GLSLParser(const std::string& source)
    : m_Source(source)
{
}

GLSLParseResult GLSLParser::Parse()
{
    std::string clean = StripComments(m_Source);

    ParseIncludes(clean);
    ParseAttributes(clean);
    ParseVaryings(clean);
    ParsePragmas(clean);
    ParseFunctions(m_Source); // Use original source for function body tracking

    return m_Result;
}

std::string GLSLParser::StripComments(const std::string& src) const
{
    std::string result;
    result.reserve(src.size());

    for (size_t i = 0; i < src.size(); ++i)
    {
        char c = src[i];
        char n = (i + 1 < src.size()) ? src[i + 1] : '\0';

        // String literal — copy verbatim
        if (c == '"')
        {
            result += c;
            i++;
            while (i < src.size() && !(src[i] == '"' && src[i - 1] != '\\'))
            {
                if (src[i] == '\n') result += '\n';
                else result += src[i];
                i++;
            }
            if (i < src.size()) { result += src[i]; }
            continue;
        }

        // Single-line comment
        if (c == '/' && n == '/')
        {
            i += 2;
            while (i < src.size() && src[i] != '\n')
                i++;
            if (i < src.size()) { result += '\n'; } // preserve newline
            continue;
        }

        // Multi-line comment
        if (c == '/' && n == '*')
        {
            i += 2;
            while (i + 1 < src.size() && !(src[i] == '*' && src[i + 1] == '/'))
            {
                if (src[i] == '\n') result += '\n';
                i++;
            }
            if (i + 1 < src.size()) i++; // skip */
            continue;
        }

        result += c;
    }

    return result;
}

void GLSLParser::ParseIncludes(const std::string& clean)
{
    static const std::regex includeRegex(R"prism(#include\s+"([^"]+)")prism");
    std::smatch match;
    std::string::const_iterator it = clean.cbegin();
    std::string::const_iterator end = clean.cend();

    while (std::regex_search(it, end, match, includeRegex))
    {
        m_Result.Includes.push_back(match[1].str());
        it = match.suffix().first;
    }
}

void GLSLParser::ParseAttributes(const std::string& clean)
{
    static const std::regex attrRegex(
        R"prism(attribute\s+(\w[\w\d]*)\s+(\w[\w\d]*)\s*:\s*(\w[\w\d]*)\s*;)prism"
    );
    std::smatch match;
    std::string::const_iterator it = clean.cbegin();
    std::string::const_iterator end = clean.cend();

    while (std::regex_search(it, end, match, attrRegex))
    {
        GLSLAttribute attr;
        attr.Type = match[1].str();
        attr.Name = match[2].str();
        attr.Semantic = match[3].str();
        m_Result.Attributes.push_back(std::move(attr));

        it = match.suffix().first;
    }
}

void GLSLParser::ParseVaryings(const std::string& clean)
{
    // Struct form: VARYING Name { members } instance;
    static const std::regex structVaryingRegex(
        R"prism(VARYING\s+(\w+)\s*\{([^}]*)\}\s*(\w+)\s*;)prism"
    );
    std::smatch match;
    std::string::const_iterator it = clean.cbegin();
    std::string::const_iterator end = clean.cend();

    while (std::regex_search(it, end, match, structVaryingRegex))
    {
        GLSLVarying varying;
        varying.IsStruct = true;
        varying.StructName = match[1].str();
        varying.Name = match[3].str();

        // Parse members from the struct body
        std::string membersBody = match[2].str();
        static const std::regex memberRegex(R"prism((\w[\w\d]*)\s+(\w[\w\d]*)\s*;)prism");
        std::smatch mMatch;
        std::string::const_iterator mIt = membersBody.cbegin();
        std::string::const_iterator mEnd = membersBody.cend();
        while (std::regex_search(mIt, mEnd, mMatch, memberRegex))
        {
            varying.Members.push_back({ mMatch[1].str(), mMatch[2].str() });
            mIt = mMatch.suffix().first;
        }

        m_Result.Varyings.push_back(std::move(varying));
        it = match.suffix().first;
    }

    // Simple form: VARYING type name;
    // Only match VARYING that wasn't already consumed by the struct form.
    // Build a new source without the struct VARYING blocks.
    std::string remaining = std::regex_replace(clean, structVaryingRegex, "");

    static const std::regex simpleVaryingRegex(
        R"prism(VARYING\s+(\w[\w\d]*)\s+(\w[\w\d]*)\s*;)prism"
    );
    it = remaining.cbegin();
    end = remaining.cend();

    while (std::regex_search(it, end, match, simpleVaryingRegex))
    {
        GLSLVarying varying;
        varying.IsStruct = false;
        varying.Type = match[1].str();
        varying.Name = match[2].str();
        m_Result.Varyings.push_back(std::move(varying));

        it = match.suffix().first;
    }
}

void GLSLParser::ParsePragmas(const std::string& clean)
{
    static const std::regex pragmaRegex(R"prism(#pragma\s+(.+))prism");
    std::smatch match;
    std::string::const_iterator it = clean.cbegin();
    std::string::const_iterator end = clean.cend();

    while (std::regex_search(it, end, match, pragmaRegex))
    {
        GLSLPragma pragma;
        pragma.Text = match[1].str();

        // Check for multi_compile / shader_feature
        static const std::regex multiCompileRegex(R"prism(multi_compile\s+(.+))prism");
        static const std::regex shaderFeatureRegex(R"prism(shader_feature\s+(.+))prism");
        std::smatch subMatch;

        if (std::regex_match(pragma.Text, subMatch, multiCompileRegex))
        {
            pragma.IsMultiCompile = true;
            std::string kwList = subMatch[1].str();
            std::istringstream ss(kwList);
            std::string kw;
            while (ss >> kw)
                pragma.Keywords.push_back(kw);
        }
        else if (std::regex_match(pragma.Text, subMatch, shaderFeatureRegex))
        {
            pragma.IsShaderFeature = true;
            std::string kwList = subMatch[1].str();
            std::istringstream ss(kwList);
            std::string kw;
            while (ss >> kw)
                pragma.Keywords.push_back(kw);
        }

        m_Result.Pragmas.push_back(std::move(pragma));
        it = match.suffix().first;
    }
}

void GLSLParser::ParseFunctions(const std::string& src)
{
    // Scan through the source looking for function definitions.
    // A function definition looks like: return_type name(params) {
    // We detect this by finding the opening { and then finding the matching }.
    // We skip comments and strings during scanning.
    // We skip variable initializations like "type name = ..." and "type name(params);"

    enum ScanState { Normal, InLineComment, InBlockComment, InString };
    ScanState state = Normal;
    size_t pos = 0;

    auto skipToEndOfLine = [&]() {
        while (pos < src.size() && src[pos] != '\n') pos++;
    };

    auto nextToken = [&]() -> std::string {
        // Skip whitespace and comments
        while (pos < src.size())
        {
            char c = src[pos];
            char n = (pos + 1 < src.size()) ? src[pos + 1] : '\0';

            if (state == InLineComment)
            {
                if (c == '\n') state = Normal;
                pos++;
                continue;
            }
            if (state == InBlockComment)
            {
                if (c == '*' && n == '/') { state = Normal; pos += 2; continue; }
                pos++;
                continue;
            }
            if (state == InString)
            {
                if (c == '"' && (pos == 0 || src[pos - 1] != '\\')) state = Normal;
                pos++;
                continue;
            }

            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { pos++; continue; }
            if (c == '/' && n == '/') { state = InLineComment; pos += 2; continue; }
            if (c == '/' && n == '*') { state = InBlockComment; pos += 2; continue; }
            if (c == '"') { state = InString; pos++; continue; }
            break;
        }
        if (pos >= src.size()) return "";

        // Read identifier or punctuation
        char c = src[pos];
        if (std::isalpha(c) || c == '_')
        {
            size_t start = pos;
            while (pos < src.size() && (std::isalnum(src[pos]) || src[pos] == '_'))
                pos++;
            return src.substr(start, pos - start);
        }
        if (c == '{' || c == '}' || c == ';' || c == '(' || c == ')' || c == ',')
        {
            pos++;
            return std::string(1, c);
        }
        // Skip other characters
        pos++;
        return "";
    };

    auto peekToken = [&]() -> std::string {
        size_t saved = pos;
        std::string t = nextToken();
        pos = saved;
        return t;
    };

    // Find function definitions: look for patterns like "type name ( params ) {"
    while (pos < src.size())
    {
        std::string token = nextToken();
        if (token.empty()) break;

        // Skip known keywords that aren't function return types
        if (token == "if" || token == "while" || token == "for" || token == "switch")
        {
            // Skip to the matching ) then look for {
            while (pos < src.size())
            {
                std::string t = nextToken();
                if (t == ")" || t == ";") break;
            }
            continue;
        }

        // Look for const as potential return type prefix
        std::string returnType = token;
        if (token == "const")
        {
            std::string next = nextToken();
            if (!next.empty())
                returnType += " " + next;
            else
                continue;
        }

        // A type followed by an identifier and then ( suggests a function
        if (returnType == "}" || returnType == ";" || returnType == ")")
            continue;

        size_t namePos = pos;
        std::string funcName = nextToken();
        if (funcName.empty()) break;

        // Skip variable declarations: type name = ... or type name;
        // Skip struct member declarations and uniform declarations
        std::string afterName = peekToken();
        if (afterName == "=" || afterName == ";")
            continue;

        // Skip if this looks like a struct definition: struct Name { ... }
        if (token == "struct")
            continue;

        // layout qualifiers
        if (token == "layout")
            continue;

        if (afterName != "(")
            continue;

        // This looks like a function declaration. Skip parameters.
        pos = namePos;
        std::string t = nextToken(); // funcName
        t = nextToken(); // should be (

        int parenDepth = 1;
        while (parenDepth > 0 && pos < src.size())
        {
            t = nextToken();
            if (t == "(") parenDepth++;
            else if (t == ")") parenDepth--;
            if (t.empty()) break;
        }

        // Check for semicolon (forward declaration) or brace (definition)
        t = peekToken();
        if (t == ";")
            continue; // Forward declaration, skip

        if (t == "{")
        {
            size_t openBrace = pos;
            nextToken(); // consume {

            // Find matching }
            int braceDepth = 1;
            size_t closeBrace = std::string::npos;
            while (braceDepth > 0 && pos < src.size())
            {
                t = nextToken();
                if (t == "{") braceDepth++;
                else if (t == "}")
                {
                    braceDepth--;
                    if (braceDepth == 0)
                        closeBrace = pos; // pos is after }
                }
            }

            GLSLFunction func;
            func.ReturnType = returnType;
            func.Name = funcName;
            func.OpenBraceOffset = openBrace;
            func.CloseBraceOffset = (closeBrace != std::string::npos) ? closeBrace : pos;
            m_Result.Functions.push_back(std::move(func));
        }
    }
}

} // namespace Prism
