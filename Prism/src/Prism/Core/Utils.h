#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

namespace Prism
{
    namespace Utils
    {
        inline std::string BytesToString(uint64_t bytes)
        {
            static const float gb = 1024 * 1024 * 1024;
            static const float mb = 1024 * 1024;
            static const float kb = 1024;

            char buffer[32];

            if (bytes > gb)
                sprintf_s(buffer, "%.2f GB", bytes / gb);
            else if (bytes > mb)
                sprintf_s(buffer, "%.2f MB", bytes / mb);
            else if (bytes > kb)
                sprintf_s(buffer, "%.2f KB", bytes / kb);
            else
                sprintf_s(buffer, "%.2f bytes", (double)bytes);

            return std::string(buffer);
        }
    }
}
