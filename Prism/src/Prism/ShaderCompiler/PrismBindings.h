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

    enum class TextureBinding : int
    {
        ShadowMap0,
        ShadowMap1,
        ShadowMap2,
        ShadowMap3,
        GeometryPassTexture,
        EnvRadiance,
        EnvIrradiance,
        EnvBRDFLUT,
        BloomTexture
    };

    constexpr int PRISM_OPENGL_SHADOW_MAP0           = 3;
    constexpr int PRISM_OPENGL_SHADOW_MAP1           = 4;
    constexpr int PRISM_OPENGL_SHADOW_MAP2           = 5;
    constexpr int PRISM_OPENGL_SHADOW_MAP3           = 6;
    constexpr int PRISM_OPENGL_GEOMETRY_PASS_TEXTURE = 7;
    constexpr int PRISM_OPENGL_ENV_RADIANCE          = 8;
    constexpr int PRISM_OPENGL_ENV_IRRADIANCE        = 9;
    constexpr int PRISM_OPENGL_ENV_BRDF_LUT          = 10;
    constexpr int PRISM_OPENGL_BLOOM_TEXTURE         = 11;

    constexpr int PRISM_VULKAN_GLOBAL_TEXTURE_SET            = 0;
    constexpr int PRISM_VULKAN_SHADOW_MAP0_BINDING           = 1;
    constexpr int PRISM_VULKAN_SHADOW_MAP1_BINDING           = 2;
    constexpr int PRISM_VULKAN_SHADOW_MAP2_BINDING           = 3;
    constexpr int PRISM_VULKAN_SHADOW_MAP3_BINDING           = 4;
    constexpr int PRISM_VULKAN_GEOMETRY_PASS_TEXTURE_BINDING = 5;
    constexpr int PRISM_VULKAN_ENV_RADIANCE_BINDING          = 6;
    constexpr int PRISM_VULKAN_ENV_IRRADIANCE_BINDING        = 7;
    constexpr int PRISM_VULKAN_ENV_BRDF_LUT_BINDING          = 8;
    constexpr int PRISM_VULKAN_BLOOM_TEXTURE_BINDING         = 9;
}
