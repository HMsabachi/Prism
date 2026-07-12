#pragma once

#include <string>

namespace Prism {

class Hash
{
public:
    static constexpr uint32_t GenerateFNVHash(std::string_view str)
    {
        constexpr uint32_t FNV_PRIME = 16777619u;
        constexpr uint32_t OFFSET_BASIS = 2166136261u;

        uint32_t hash = OFFSET_BASIS;
        for (char c : str)
        {
            hash ^= static_cast<uint32_t>(c);
            hash *= FNV_PRIME;
        }

        return hash;
    }

    static constexpr uint64_t GenerateFNVHash64(std::string_view str)
    {
        constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        constexpr uint64_t OFFSET_BASIS = 14695981039346656037ULL;

        uint64_t hash = OFFSET_BASIS;
        for (char c : str)
        {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_PRIME;
        }

        return hash;
    }
};

}
