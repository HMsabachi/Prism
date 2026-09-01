#include "prpch.h"
#include "TextureUtils.h"

#include "Prism/Utilities/Utilities.h"
#include "stb_image.h"
#include "stb_image_write.h"

#include <cctype>
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

        uint32_t ReadU32(const byte* p, size_t off)
        {
            uint32_t v;
            memcpy(&v, p + off, sizeof(v));
            return v;
        }

        bool MapDXGIFormat(uint32_t dxgi, ImageFormat& format)
        {
            switch (dxgi)
            {
                case 71: format = ImageFormat::BC1; return true;
                case 72: format = ImageFormat::BC1SRGB; return true;
                case 74: format = ImageFormat::BC2; return true;
                case 75: format = ImageFormat::BC2SRGB; return true;
                case 77: format = ImageFormat::BC3; return true;
                case 78: format = ImageFormat::BC3SRGB; return true;
                case 80: format = ImageFormat::BC4; return true;
                case 83: format = ImageFormat::BC5; return true;
                case 95: format = ImageFormat::BC6H; return true;
                case 98: format = ImageFormat::BC7; return true;
                case 99: format = ImageFormat::BC7SRGB; return true;
                default: return false;
            }
        }

        bool MapFourCC(uint32_t fourcc, ImageFormat& format)
        {
            switch (fourcc)
            {
                case 0x31545844: format = ImageFormat::BC1; return true; // DXT1
                case 0x33545844: format = ImageFormat::BC2; return true; // DXT3
                case 0x35545844: format = ImageFormat::BC3; return true; // DXT5
                case 0x31495441: format = ImageFormat::BC4; return true; // ATI1
                case 0x32495441: format = ImageFormat::BC5; return true; // ATI2
                case 0x55344342: format = ImageFormat::BC4; return true; // BC4U
                case 0x55354342: format = ImageFormat::BC5; return true; // BC5U
                default: return false;
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

    bool IsDDSFile(const std::string& path)
    {
        std::string ext = std::filesystem::path(path).extension().string();
        for (char& c : ext)
            c = (char)std::tolower((unsigned char)c);
        return ext == ".dds";
    }

    bool LoadDDS(const std::string& path, TextureLoadResult& out)
    {
        std::string file = File::ReadFile(path);
        if (file.size() < 128)
            return false;

        const byte* data = (const byte*)file.data();

        if (memcmp(data, "DDS ", 4) != 0)
            return false;

        uint32_t height = ReadU32(data, 12);
        uint32_t width = ReadU32(data, 16);
        uint32_t mipCount = ReadU32(data, 28);
        if (mipCount == 0)
            mipCount = 1;
        if (width == 0 || height == 0)
            return false;

        uint32_t fourCC = ReadU32(data, 84);
        ImageFormat format = ImageFormat::None;

        size_t dataOffset = 128;
        if (fourCC == 0x30315844) // DX10
        {
            uint32_t dxgi = ReadU32(data, 128);
            if (!MapDXGIFormat(dxgi, format))
            {
                PR_CORE_ERROR("Unsupported DDS DXGI format {0}: {1}", dxgi, path);
                return false;
            }
            dataOffset = 148;
        }
        else if (!MapFourCC(fourCC, format))
        {
            PR_CORE_ERROR("Unsupported DDS FourCC 0x{0:X}: {1}", fourCC, path);
            return false;
        }

        uint32_t blockSize = Utils::GetCompressedBlockSize(format);

        out.Format = format;
        out.Width = width;
        out.Height = height;
        out.Mips.clear();
        out.Mips.reserve(mipCount);

        size_t offset = dataOffset;
        for (uint32_t i = 0; i < mipCount; i++)
        {
            uint32_t w = std::max(1u, width >> i);
            uint32_t h = std::max(1u, height >> i);
            uint32_t size = ((w + 3) / 4) * ((h + 3) / 4) * blockSize;
            if (offset + size > file.size())
            {
                PR_CORE_ERROR("DDS data truncated: {0}", path);
                return false;
            }
            out.Mips.push_back(Buffer::Copy(data + offset, size));
            offset += size;
        }

        return true;
    }
}
