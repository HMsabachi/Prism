#pragma once

#include "Prism/Asset/Asset.h"
#include <filesystem>

namespace Prism
{

    enum class AssetStatus
    {
        None    = 0,
        Ready   = 1,
        Invalid = 2,
        Loading = 3
    };

    struct AssetMetadata
    {
        AssetHandle Handle = 0;
        AssetType Type;
        std::filesystem::path FilePath;
        AssetStatus Status = AssetStatus::None;
        uint64_t FileLastWriteTime = 0;
        bool IsDataLoaded = false;

        bool IsValid() const { return Handle != 0; }
    };

}
