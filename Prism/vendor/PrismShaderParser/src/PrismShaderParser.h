#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <filesystem>
#include <fstream>
#include <set>

#ifdef PR_PLATFORM_WINDOWS
    #if PR_DYNAMIC_LINK
        #ifdef PR_BUILD_DLL
            #define PRISM_API __declspec(dllexport)
        #else
            #define PRISM_API __declspec(dllimport)
        #endif
    #else
        #define PRISM_API
    #endif
#else
    #error Prism only supports Windows!
#endif

namespace Prism
{
    class Application;
}

namespace Prism
{
    // 属性类型
    enum class PRISM_API PropertyType
    {
        Float,
        Int,
        Color,      // vec4
        Vector2,
        Vector3,
        Vector4,
        Texture2D,
        Range       // Float + [min, max]
    };

    // 单个属性的完整信息
    struct PRISM_API PropertyDescriptor
    {
        std::string Name;           // _MainColor
        std::string DisplayName;    // "主颜色"
        PropertyType Type;
        std::string DefaultValue;   // "(1,1,1,1)" 或 "0.5" 或 "white"
        float Min = 0.0f;           // Range 用
        float Max = 1.0f;
    };
    // 顶点语义枚举（映射到底层的 location）
    enum class PRISM_API VertexSemantic
    {
        Position,   // 对应 location = 0
        Normal,     // 对应 location = 1
        TexCoord0,  // 对应 location = 2
        Color,      // 对应 location = 3
        Tangent,    // 对应 location = 4
        Unknown
    };
    // 单个顶点属性的描述
    struct PRISM_API VertexAttributeDescriptor
    {
        std::string Name;           // 例如 "aPos"
        std::string Type;           // 例如 "vec3"
        std::string SemanticStr;    // 例如 "POSITION"
        VertexSemantic Semantic;    // 枚举化的语义
        int Location = 0;           // 引擎最终分配的槽位号
    };
    struct PRISM_API PassDescriptor
    {
        std::string Name;
        std::unordered_map<std::string, std::string> Tags;  // "Queue" = "Geometry"
        std::vector<VertexAttributeDescriptor> Attributes;
        std::string RawGLSL;        // 用户在 GLSL { } 里写的原始代码
        std::string ProcessedGLSL;  // 经过预处理（注入全局+include）后的 GLSL

        // --- 拆分后的最终代码 ---
        std::string VertexShaderCode;
        std::string FragmentShaderCode;
    };

    // 解析结果
    struct PRISM_API ParseResult
    {
        std::string ShaderName;
        std::vector<PropertyDescriptor> Properties;
        std::string RawGLSL;        // 用户在 GLSL { } 里写的原始代码
        std::string ProcessedGLSL;  // 经过预处理（注入全局+include）后的 GLSL
        std::vector<PassDescriptor> Passes;
    };

    // Prism Shader 解析器（目前只处理字符串，不涉及文件IO）
    class PRISM_API PrismShaderParser
    {
        friend class Prism::Application;
    private:
    #pragma region 类型转换
        // 将字符串类型转换为枚举
        static PropertyType StringToPropertyType(const std::string& typeStr, float& outMin, float& outMax);
        // 将枚举类型转换为字符串
        static std::string PropertyTypeToString(PropertyType type);
    #pragma endregion

    public:
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
        inline static int GetLocationBySemantic(const std::string& semantic, Prism::VertexSemantic& outSemantic) {
            if (semantic == "POSITION") { outSemantic = VertexSemantic::Position; return static_cast<int>(outSemantic); }
            if (semantic == "NORMAL") { outSemantic = VertexSemantic::Normal;   return static_cast<int>(outSemantic); }
            if (semantic == "TEXCOORD0") { outSemantic = VertexSemantic::TexCoord0; return static_cast<int>(outSemantic); }
            if (semantic == "COLOR") { outSemantic = VertexSemantic::Color;    return static_cast<int>(outSemantic); }
            if (semantic == "TANGENT") { outSemantic = VertexSemantic::Tangent;  return static_cast<int>(outSemantic); }

            outSemantic = VertexSemantic::Unknown;
            return -1;
        }
        static void ProcessAttributes(PassDescriptor& pass);
        static void InjectHeader(PassDescriptor& pass, const ParseResult& result);
        static void SplitShader(PassDescriptor& pass);
        static void RemoveFunction(std::string& code, const std::string& funcName);
        #pragma endregion

        #pragma region 后处理
        static std::string FormatGLSL(const std::string& code);
        static void FormatCodeInPlace(std::string& code);
    #pragma endregion

    };
        
}

