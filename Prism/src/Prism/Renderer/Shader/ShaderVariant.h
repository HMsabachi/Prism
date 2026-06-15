#pragma once
#include "Prism/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Prism
{
    class Shader;

    using KeywordMask = uint64_t;
    constexpr size_t MAX_KEYWORDS_PER_SHADER = 64;

    struct ShaderKeyword
    {
        std::string Name;
        uint8_t Index = 0; // bit position (0-63)
    };

    struct ShaderVariant
    {
        KeywordMask Mask = 0;
        std::string DebugName; // e.g. "ALBEDO_MAP|NORMAL_MAP"
        Ref<Shader> ShaderProgram;          // 单 Pass 兼容（等价于 PassPrograms[0]）
        std::vector<Ref<Shader>> PassPrograms; // 多 Pass：每个 Pass 的变体版本
    };
}
