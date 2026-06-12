#pragma once

#include <cstdint>

namespace Prism::PSL
{

    enum class PropertyType : uint8_t
    {
        None,
        Bool,
        Color,
        Color3,
        Float,
        Int,
        Vector2,
        Vector3,
        Vector4,
        Range,
        Matrix3,
        Matrix4,
        Texture2D,
        Texture2DMS,
        TextureCube,
        Enum,
    };

    namespace PropertyTypeUtil
    {

        // std140 对齐字节数
        uint32_t Alignment(PropertyType type);

        // std140 大小
        uint32_t Size(PropertyType type);

        // 属性类型 → GLSL uniform 声明 (如 "uniform float", "uniform sampler2D")
        const char* ToGLSLUniform(PropertyType type);

        // 属性类型 → GLSL 类型名 (如 "float", "vec4", "sampler2D")
        const char* ToGLSLType(PropertyType type);

    } // namespace PropertyTypeUtil

} // namespace Prism::PSL
