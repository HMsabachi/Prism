#pragma once
#include "spdlog/fmt/fmt.h"
#include <xhash>
namespace Prism
{
    class PRISM_API UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID& other);

        operator uint64_t () { return m_UUID; }
        operator const uint64_t() const { return m_UUID; }
    private:
        uint64_t m_UUID;
    };
    inline auto format_as(const UUID& uuid) { return static_cast<uint64_t>(uuid); }
}
namespace std {

    template <>
    struct hash<Prism::UUID>
    {
        std::size_t operator()(const Prism::UUID& uuid) const
        {
            return hash<uint64_t>()((uint64_t)uuid);
        }
    };
}
