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
    // 属性类型
    enum class PRISM_API ParserPropertyType
    {
        Float,
        Int,
        Color,      // vec4
        Vector2,
        Vector3,
        Vector4,
        Texture2D,
        Texture2DMS,
        TextureCube,
        Range,       // Float + [min, max]
        Matrix3, 
		Matrix4
    };

    // 单个属性的完整信息
    struct PRISM_API PropertyDescriptor
    {
        std::string Name;           // _MainColor
        std::string DisplayName;    // "主颜色"
        ParserPropertyType Type;
        std::string DefaultValue;   // "(1,1,1,1)" 或 "0.5" 或 "white"
        float Min = 0.0f;           // Range 用
        float Max = 1.0f;
    };
    // 顶点语义枚举（映射到底层的 location）
    enum class PRISM_API VertexSemantic
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
    inline uint32_t VertexSemanticToLocation(VertexSemantic semantic)
    {
		return static_cast<uint32_t>(semantic);
	}
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
        std::vector<PassDescriptor> Passes;
        std::string RenderCommand; 
		bool Success = false;
    };
}