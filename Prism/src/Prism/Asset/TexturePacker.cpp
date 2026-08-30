#include "prpch.h"
#include "TexturePacker.h"

#include "stb_image.h"
#include "stb_image_write.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace Prism
{
    namespace
    {
        uint8_t SampleChannel(const uint8_t* px, int channels, int ch)
        {
            if (channels <= 2) return px[0];
            return px[ch];
        }
        void ResizeChannel(const uint8_t* src, int srcW, int srcH, uint8_t* dst, int dstW, int dstH)
        {
            if (srcW == dstW && srcH == dstH)
            {
                std::memcpy(dst, src, (size_t)srcW * srcH);
                return;
            }

            const float sx = (float)srcW / dstW;
            const float sy = (float)srcH / dstH;
            for (int y = 0; y < dstH; y++)
            {
                float fy = (y + 0.5f) * sy - 0.5f;
                if (fy < 0.0f) fy = 0.0f;
                int y0 = (int)fy;
                int y1 = y0 + 1 < srcH ? y0 + 1 : srcH - 1;
                float ty = fy - y0;

                for (int x = 0; x < dstW; x++)
                {
                    float fx = (x + 0.5f) * sx - 0.5f;
                    if (fx < 0.0f) fx = 0.0f;
                    int x0 = (int)fx;
                    int x1 = x0 + 1 < srcW ? x0 + 1 : srcW - 1;
                    float tx = fx - x0;

                    float p00 = src[y0 * srcW + x0];
                    float p01 = src[y0 * srcW + x1];
                    float p10 = src[y1 * srcW + x0];
                    float p11 = src[y1 * srcW + x1];

                    float top = p00 + (p01 - p00) * tx;
                    float bot = p10 + (p11 - p10) * tx;
                    dst[y * dstW + x] = (uint8_t)(top + (bot - top) * ty + 0.5f);
                }
            }
        }
    }

    bool PackOrmTexture(const std::vector<OrmPackSource>& sources, const std::string& ormPath)
    {
        if (sources.empty() || ormPath.empty())
            return false;
        int dstW = 1, dstH = 1;
        for (const auto& s : sources)
        {
            int w = 0, h = 0, c = 0;
            if (stbi_info(s.Path.c_str(), &w, &h, &c))
            {
                dstW = std::max(dstW, w);
                dstH = std::max(dstH, h);
            }
        }
        std::vector<uint8_t> dst((size_t)dstW * dstH * 3);
        for (size_t i = 0; i < (size_t)dstW * dstH; i++)
        {
            dst[i * 3 + 0] = 255;
            dst[i * 3 + 1] = 255;
            dst[i * 3 + 2] = 0;
        }
        for (const auto& s : sources)
        {
            int w = 0, h = 0, c = 0;
            uint8_t* data = stbi_load(s.Path.c_str(), &w, &h, &c, 0);
            if (!data)
                continue;

            std::vector<uint8_t> chan((size_t)w * h);
            for (size_t i = 0; i < (size_t)w * h; i++)
                chan[i] = SampleChannel(data + i * c, c, s.SrcChannel);

            std::vector<uint8_t> scaled((size_t)dstW * dstH);
            ResizeChannel(chan.data(), w, h, scaled.data(), dstW, dstH);

            if (s.DstChannel >= 0 && s.DstChannel <= 2)
                for (size_t i = 0; i < (size_t)dstW * dstH; i++)
                    dst[i * 3 + s.DstChannel] = scaled[i];

            stbi_image_free(data);
        }
        return stbi_write_tga(ormPath.c_str(), dstW, dstH, 3, dst.data()) != 0;
    }
}
