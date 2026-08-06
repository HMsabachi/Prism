#pragma once

namespace Prism::Config
{
    constexpr int PRISM_OPENGL_BINDING_FRAME    = 0;
    constexpr int PRISM_OPENGL_BINDING_OBJECT   = 1;
    constexpr int PRISM_OPENGL_BINDING_MATERIAL = 2;

    constexpr int PRISM_VULKAN_BINDING_FRAME    = 0;
    constexpr int PRISM_VULKAN_BINDING_OBJECT   = 0;
    constexpr int PRISM_VULKAN_BINDING_MATERIAL = 0;

    constexpr int PRISM_OPENGL_TEXTURE_BEGIN_BINDING = 16;
    constexpr int PRISM_VULKAN_TEXTURE_BEGIN_SET     = 2;
    constexpr int PRISM_VULKAN_TEXTURE_BEGIN_BINDING = 1;

    // 全局 texture 逻辑槽位(给 SetGlobalTexture 用)
    constexpr int PRISM_SHADOW_MAP0_SLOT           = 0;
    constexpr int PRISM_SHADOW_MAP1_SLOT           = 1;
    constexpr int PRISM_SHADOW_MAP2_SLOT           = 2;
    constexpr int PRISM_SHADOW_MAP3_SLOT           = 3;
    constexpr int PRISM_GEOMETRY_PASS_TEXTURE_SLOT = 4;
    constexpr int PRISM_ENV_RADIANCE_SLOT          = 5;
    constexpr int PRISM_ENV_IRRADIANCE_SLOT        = 6;
    constexpr int PRISM_ENV_BRDF_LUT_SLOT          = 7;
    constexpr int PRISM_BLOOM_TEXTURE_SLOT         = 8;

    // 全局 texture 实际起始 binding
    constexpr int PRISM_OPENGL_GLOBAL_TEXTURE_BEGIN = 3;
    constexpr int PRISM_VULKAN_GLOBAL_TEXTURE_SET   = 0;
    constexpr int PRISM_VULKAN_GLOBAL_TEXTURE_BEGIN = 1;
}
