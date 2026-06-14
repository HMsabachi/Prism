#pragma once

#include "PropertyType.h"

#include <string>
#include <vector>

namespace Prism::PSL
{

class PropertyLayout
{
public:
    struct Member
    {
        std::string Name;
        PropertyType Type;
        uint32_t Offset = 0;
        uint32_t Size = 0;
        uint32_t ArrayCount = 1;
    };

    PropertyLayout& Add(const std::string& name, PropertyType type, uint32_t count = 1);

    const Member* Find(const std::string& name) const;
    const std::vector<Member>& GetMembers() const { return m_Members; }
    uint32_t GetMaxAlignment() const { return m_MaxAlignment; }
    uint32_t GetTotalSize() const {
        if (m_MaxAlignment == 0) return m_TotalSize;
        uint32_t size = m_TotalSize;
        uint32_t rem = size % m_MaxAlignment;
        return (rem == 0) ? size : size + m_MaxAlignment - rem;
    }
    bool IsEmpty() const { return m_Members.empty(); }

    using const_iterator = std::vector<Member>::const_iterator;
    const_iterator begin() const { return m_Members.begin(); }
    const_iterator end() const { return m_Members.end(); }

private:
    std::vector<Member> m_Members;
    uint32_t m_TotalSize = 0;
    uint32_t m_MaxAlignment = 0;
};

} // namespace Prism::PSL
