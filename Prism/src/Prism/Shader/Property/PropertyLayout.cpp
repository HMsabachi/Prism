#include "prpch.h"
#include "PropertyLayout.h"

namespace Prism::PSL
{

PropertyLayout& PropertyLayout::Add(const std::string& name, PropertyType type, uint32_t count)
{
    // std140 对齐偏移
    uint32_t align = PropertyTypeUtil::Alignment(type);
    if (align > m_MaxAlignment)
        m_MaxAlignment = align;
    uint32_t offset = m_TotalSize;
    if (offset % align != 0)
        offset += align - (offset % align);

    uint32_t size = PropertyTypeUtil::Size(type) * count;

    Member member;
    member.Name = name;
    member.Type = type;
    member.Offset = offset;
    member.Size = size;
    member.ArrayCount = count;
    m_Members.push_back(member);

    m_TotalSize = offset + size;
    return *this;
}

const PropertyLayout::Member* PropertyLayout::Find(const std::string& name) const
{
    for (auto& m : m_Members)
        if (m.Name == name)
            return &m;
    return nullptr;
}

} // namespace Prism::PSL
