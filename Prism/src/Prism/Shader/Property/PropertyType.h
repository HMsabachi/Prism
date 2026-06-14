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

        uint32_t Alignment(PropertyType type);

        uint32_t Size(PropertyType type);

        const char* ToGLSLUniform(PropertyType type);

        const char* ToGLSLType(PropertyType type);

        inline bool IsTextureType(PropertyType type)
        {
            return type == PropertyType::Texture2D
                || type == PropertyType::Texture2DMS
                || type == PropertyType::TextureCube;
        }

    } // namespace PropertyTypeUtil

} // namespace Prism::PSL
