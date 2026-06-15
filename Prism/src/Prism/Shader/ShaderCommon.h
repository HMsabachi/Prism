#pragma once

#include <cstdint>
#include <string>

namespace Prism::PSL
{

    struct SourceLocation
    {
        uint32_t Line = 0;
        uint32_t Column = 0;
        uint32_t Offset = 0;
        std::string_view FilePath;
    };

    enum class ShaderStageType : uint8_t
    {
        Vertex,
        Fragment,
        Compute,
    };

} // namespace Prism::PSL
