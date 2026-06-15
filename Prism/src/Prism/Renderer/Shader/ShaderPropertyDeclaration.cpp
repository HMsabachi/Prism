#include "prpch.h"
#include "ShaderPropertyDeclaration.h"
#include "Prism/Renderer/Renderer.h"

namespace Prism
{
#pragma region PropertyType

    PropertyType::Texture2D::operator bool() const { return texture; }
    PropertyType::Texture2D::operator uint32_t() const { return slot; }
    PropertyType::Texture2D::operator Ref<Prism::Texture2D>() const { return texture; }
    void PropertyType::Texture2D::SetTexture(Ref<Prism::Texture2D>& t) { texture = t; }
    std::string PropertyType::Texture2D::ToString() const { return fmt::format("Texture2D(Slot: {})", slot); }

    PropertyType::TextureCube::operator bool() const { return texture; }
    PropertyType::TextureCube::operator uint32_t() const { return slot; }
    PropertyType::TextureCube::operator Ref<Prism::TextureCube>() const { return texture; }
    void PropertyType::TextureCube::SetTexture(Ref<Prism::TextureCube>& t) { texture = t; }
    std::string PropertyType::TextureCube::ToString() const { return fmt::format("TextureCube(Slot: {})", slot); }

#pragma endregion

#pragma region PropertyDeclaration

    PropertyDeclaration::PropertyDeclaration(PropertyDeclarationType type, const std::string& name, const std::string displayName, uint32_t count)
        : m_Name(name), m_DisplayName(displayName), m_Count(count), m_Type(type), m_Size(SizeOfType(type) * count)
    {
    }

    uint32_t PropertyDeclaration::SizeOfType(PropertyDeclarationType type)
    {
        switch (type)
        {
        case PropertyDeclarationType::Bool:		return sizeof(PropertyType::Bool);
        case PropertyDeclarationType::Color:		return sizeof(PropertyType::Color);
        case PropertyDeclarationType::Color3:	return sizeof(PropertyType::Vector3);
        case PropertyDeclarationType::Enum:		return sizeof(PropertyType::Int);
        case PropertyDeclarationType::Float:		return sizeof(PropertyType::Float);
        case PropertyDeclarationType::Int:			return sizeof(PropertyType::Int);
        case PropertyDeclarationType::Vector2:		return sizeof(PropertyType::Vector2);
        case PropertyDeclarationType::Vector3:		return sizeof(PropertyType::Vector3);
        case PropertyDeclarationType::Vector4:		return sizeof(PropertyType::Vector4);
        case PropertyDeclarationType::Range:		return sizeof(PropertyType::Range);
        case PropertyDeclarationType::Texture2D:	return sizeof(PropertyType::Texture2D);
        case PropertyDeclarationType::Texture2DMS:	return sizeof(PropertyType::Texture2D);
        case PropertyDeclarationType::TextureCube:	return sizeof(PropertyType::TextureCube);
        case PropertyDeclarationType::Matrix3:		return sizeof(PropertyType::Matrix3);
        case PropertyDeclarationType::Matrix4:		return sizeof(PropertyType::Matrix4);
        default:									return 0;
        }
    }

#pragma endregion

#pragma region PropertyBufferDeclaration

    PropertyDeclaration& PropertyBufferDeclaration::AddProperty(const PropertyDeclaration& property)
    {
        uint32_t offset = 0;
        if (!m_Properties.empty())
        {
            const PropertyDeclaration& previous = m_Properties.back();
            offset = previous.GetOffset() + previous.GetSize();
        }

        m_Properties.push_back(property);
        m_Properties.back().SetOffset(offset);
        m_Size += property.GetSize();
        return m_Properties.back();
    }

    const PropertyDeclaration* PropertyBufferDeclaration::FindProperty(const std::string& name) const
    {
        for (const auto& prop : m_Properties)
        {
            if (prop.GetName() == name)
                return &prop;
        }
        return nullptr;
    }

#pragma endregion
}
