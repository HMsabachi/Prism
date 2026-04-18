#pragma once


#include "PrismShaderData.h"

namespace Prism
{
    class Application;
}
namespace Prism
{
    // Prism Shader 解析器（目前只处理字符串，不涉及文件IO）
    class PRISM_API PrismShaderParser
    {
        friend class Prism::Application;
    private:
    #pragma region 类型转换
        static ParserPropertyType StringToPropertyType(const std::string& typeStr, float& outMin, float& outMax);
        static std::string PropertyTypeToString(ParserPropertyType type);
        static int GetLocationBySemantic(const std::string& semantic, Prism::VertexSemantic& outSemantic);
    #pragma endregion

    public:
		static void Init();
        ParseResult Parse(const std::string& source);
    private:
        static void SetIncludeRoot(const std::string& root) { s_IncludeRoot = root; }
        static std::string GetIncludeRoot() { return s_IncludeRoot; }
        static std::string s_IncludeRoot;
        static std::string s_VersionHeader;
        static std::string s_FileHeader;
    private:
        #pragma region 初步处理
        // 移除源代码中的所有单行和多行注释，并规范化换行符。
        static std::string StripComments(const std::string& source);

        // 提取 Shader "Name"
        static bool ExtractShaderMetadata(const std::string& source, ParseResult& outResult);

        // 提取并解析 Properties { ... } 块
        static bool ParsePropertiesBlock(const std::string& source, ParseResult& outResult);

        // 解析 SubShader { ... } 块
        static bool ParseSubShader(const std::string& source, ParseResult& outResult);

        // 解析 Pass { ... } 块
        static void ParsePassInternal(const std::string& passContent, PassDescriptor& outPass);

        // 处理#include指令，递归处理依赖文件
        static std::string ResolveIncludes(
            const std::string& source,
            const std::filesystem::path& includeRoot,
            std::set<std::filesystem::path>& includeHistory);
    
        #pragma endregion

        #pragma region 分离翻译为GLSL
        static void ProcessAttributes(PassDescriptor& pass);
        static void InjectHeader(PassDescriptor& pass, const ParseResult& result);
        static void SplitShader(PassDescriptor& pass);
        static void RemoveFunction(std::string& code, const std::string& funcName);
        #pragma endregion

        #pragma region 后处理
        static std::string FormatGLSL(const std::string& code);
        static void FormatCodeInPlace(std::string& code);
    #pragma endregion

        bool ParseRenderCommand(std::string& cleanCode, ParseResult& result);
    };
        
}

