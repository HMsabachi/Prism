#pragma once

#include "Prism/Renderer/Image.h"

#include <string>
#include <vector>

namespace Prism
{
    struct OrmPackSource
    {
        std::string Path;
        int SrcChannel = 0;
        int DstChannel = 0;
    };

    struct TextureLoadResult
    {
        ImageFormat Format = ImageFormat::None;
        uint32_t Width = 0;
        uint32_t Height = 0;
        std::vector<Buffer> Mips;
    };

    bool PackOrmTexture(const std::vector<OrmPackSource>& sources, const std::string& ormPath);

    bool LoadDDS(const std::string& path, TextureLoadResult& out);
    bool IsDDSFile(const std::string& path);
}
