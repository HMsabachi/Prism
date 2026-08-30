#pragma once

#include <string>
#include <vector>

namespace Prism
{
    struct OrmPackSource
    {
        std::string Path;
        int SrcChannel = 0; // 源图通道索引：0=R 1=G 2=B（灰度图视为 R）
        int DstChannel = 0; // ORM 目标通道：0=R(AO) 1=G(roughness) 2=B(metalness)
    };

    // 把若干灰度源图合并成一张 RGB ORM 图并落盘为 TGA 缓存
    bool PackOrmTexture(const std::vector<OrmPackSource>& sources, const std::string& ormPath);
}
