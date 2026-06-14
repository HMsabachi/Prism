#include "prpch.h"
#include "PropertyType.h"

#include <cassert>

namespace Prism::PSL::PropertyTypeUtil
{

uint32_t Alignment(PropertyType type)
{
    switch (type)
    {
    case PropertyType::Bool:
    case PropertyType::Float:
    case PropertyType::Int:
    case PropertyType::Enum:
        return 4;
    case PropertyType::Vector2:
        return 8;
    case PropertyType::Color:
    case PropertyType::Color3:
    case PropertyType::Vector3:
    case PropertyType::Vector4:
        return 16;
    case PropertyType::Range:
        return 4;
    case PropertyType::Matrix3:
        return 16; // 3×vec4 → 48 bytes, aligned 16
    case PropertyType::Matrix4:
        return 16; // 4×vec4 → 64 bytes
    case PropertyType::Texture2D:
    case PropertyType::Texture2DMS:
    case PropertyType::TextureCube:
        return 4; // sampler 不在 UBO 内，slot 是 int
    default:
        return 4;
    }
}

uint32_t Size(PropertyType type)
{
    switch (type)
    {
    case PropertyType::Bool:   return 4;
    case PropertyType::Float:  return 4;
    case PropertyType::Int:    return 4;
    case PropertyType::Enum:   return 4;
    case PropertyType::Vector2: return 8;
    case PropertyType::Color3:
    case PropertyType::Vector3: return 12; // std140: vec3 = 12
    case PropertyType::Color:
    case PropertyType::Vector4: return 16;
    case PropertyType::Range:   return 4;
    case PropertyType::Matrix3: return 48; // 3×vec4
    case PropertyType::Matrix4: return 64; // 4×vec4
    case PropertyType::Texture2D:
    case PropertyType::Texture2DMS:
    case PropertyType::TextureCube: return 4; // slot int
    default: return 0;
    }
}

const char* ToGLSLUniform(PropertyType type)
{
    switch (type)
    {
    case PropertyType::Bool:       return "uniform bool";
    case PropertyType::Float:      return "uniform float";
    case PropertyType::Int:        return "uniform int";
    case PropertyType::Enum:       return "uniform int";
    case PropertyType::Vector2:    return "uniform vec2";
    case PropertyType::Color3:
    case PropertyType::Vector3:    return "uniform vec3";
    case PropertyType::Color:
    case PropertyType::Vector4:    return "uniform vec4";
    case PropertyType::Range:      return "uniform float";
    case PropertyType::Matrix3:    return "uniform mat3";
    case PropertyType::Matrix4:    return "uniform mat4";
    case PropertyType::Texture2D:  return "uniform sampler2D";
    case PropertyType::Texture2DMS: return "uniform sampler2DMS";
    case PropertyType::TextureCube: return "uniform samplerCube";
    default: return "uniform float";
    }
}

const char* ToGLSLType(PropertyType type)
{
    switch (type)
    {
    case PropertyType::Bool:       return "bool";
    case PropertyType::Float:      return "float";
    case PropertyType::Int:
    case PropertyType::Enum:       return "int";
    case PropertyType::Vector2:    return "vec2";
    case PropertyType::Color3:
    case PropertyType::Vector3:    return "vec3";
    case PropertyType::Color:
    case PropertyType::Vector4:    return "vec4";
    case PropertyType::Matrix3:    return "mat3";
    case PropertyType::Matrix4:    return "mat4";
    case PropertyType::Texture2D:  return "sampler2D";
    case PropertyType::Texture2DMS: return "sampler2DMS";
    case PropertyType::TextureCube: return "samplerCube";
    case PropertyType::Range:      return "float";
    default: return "float";
    }
}

} // namespace Prism::PSL::PropertyTypeUtil
