#pragma once

#include <cstdint>

namespace Prism::Config
{
    //==================================================
    // Set 编号(逻辑,后端无关)
    // 0 FRAME       帧级 UBO(camera/time/screen)
    // 1 RENDER_PASS pass 产物(shadow/env/bloom/几何 pass 纹理)
    // 2 TRANSFORMS  per-instance object UBO(dynamic offset)
    // 3 MATERIAL    材质 UBO + 材质纹理
    //==================================================
    constexpr uint32_t PRISM_SET_FRAME        = 0;
    constexpr uint32_t PRISM_SET_RENDER_PASS  = 1;
    constexpr uint32_t PRISM_SET_TRANSFORMS  = 2;
    constexpr uint32_t PRISM_SET_MATERIAL     = 3;

    //==================================================
    // OpenGL 物理号 BASE 表
    // GL 有 4 个独立编号空间(UBO/SSBO/Texture/Image),各自从 0 起。
    // flat(type, set, binding) = BASE[type][set] + binding
    //==================================================

    //---- UBO 空间(GL_UNIFORM_BUFFER, 84 个)----
    constexpr uint32_t GL_UBO_BASE_FRAME        = 0;
    constexpr uint32_t GL_UBO_BASE_RENDER_PASS  = 4;
    constexpr uint32_t GL_UBO_BASE_TRANSFORMS   = 16;
    constexpr uint32_t GL_UBO_BASE_MATERIAL     = 20;

    //---- SSBO 空间(GL_SHADER_STORAGE_BUFFER, 84 个)----
    constexpr uint32_t GL_SSBO_BASE_FRAME       = 0;
    constexpr uint32_t GL_SSBO_BASE_RENDER_PASS = 4;
    constexpr uint32_t GL_SSBO_BASE_TRANSFORMS  = 16;
    constexpr uint32_t GL_SSBO_BASE_MATERIAL    = 20;

    //---- Texture 空间(32 档, GL_MAX_TEXTURE_IMAGE_UNITS >= 32)----
    constexpr uint32_t GL_TEX_BASE_FRAME        = 0;
    constexpr uint32_t GL_TEX_BASE_RENDER_PASS  = 1;
    constexpr uint32_t GL_TEX_BASE_MATERIAL     = 12;

    //---- Texture 空间(16 档保底)----
    constexpr uint32_t GL_TEX16_BASE_FRAME       = 0;
    constexpr uint32_t GL_TEX16_BASE_RENDER_PASS = 1;
    constexpr uint32_t GL_TEX16_BASE_MATERIAL    = 10;

    //---- Image 空间(image unit, 8 个)----
    constexpr uint32_t GL_IMG_BASE_FRAME        = 0;
    constexpr uint32_t GL_IMG_BASE_RENDER_PASS  = 1;
    constexpr uint32_t GL_IMG_BASE_MATERIAL     = 4;

    //==================================================
    // 旧宏名别名(值=新分段表对应位,给编译器 config 用)
    //==================================================

    // UBO(材质 UBO 编译器生成的 layout binding)
    constexpr uint32_t PRISM_OPENGL_BINDING_MATERIAL = GL_UBO_BASE_MATERIAL;    // 20 set3

    // Texture(MATERIAL 段, set3)
    constexpr uint32_t PRISM_OPENGL_TEXTURE_BEGIN_BINDING = GL_TEX_BASE_MATERIAL;  // 12

    // 全局 texture 逻辑槽位(set1 RENDER_PASS 内 binding)
    // per-pass 复用:每个 pass 的槽位都从 0 数,不跨 pass 独占。
    // GeometryPass:shadow 0-3 + env 4-6 (共 7 个)
    constexpr uint32_t PRISM_GEOMETRY_SHADOW_MAP0_SLOT = 0;
    constexpr uint32_t PRISM_GEOMETRY_SHADOW_MAP1_SLOT = 1;
    constexpr uint32_t PRISM_GEOMETRY_SHADOW_MAP2_SLOT = 2;
    constexpr uint32_t PRISM_GEOMETRY_SHADOW_MAP3_SLOT = 3;
    constexpr uint32_t PRISM_GEOMETRY_ENV_RADIANCE_SLOT  = 4;
    constexpr uint32_t PRISM_GEOMETRY_ENV_IRRADIANCE_SLOT = 5;
    constexpr uint32_t PRISM_GEOMETRY_ENV_BRDF_LUT_SLOT  = 6;

    // CompositePass:geo color (1 个)
    constexpr uint32_t PRISM_COMPOSITE_GEOMETRY_COLOR_SLOT = 0;

    // BloomBlurPass:blur input (1 个)
    constexpr uint32_t PRISM_BLOOM_BLUR_INPUT_SLOT = 0;

    // BloomBlendPass:geo color + bloom (2 个)
    constexpr uint32_t PRISM_BLOOM_BLEND_GEOMETRY_COLOR_SLOT = 0;
    constexpr uint32_t PRISM_BLOOM_BLEND_BLOOM_SLOT = 1;

    //==================================================
    // Vulkan(直通, set/binding)
    //==================================================
    constexpr uint32_t PRISM_VULKAN_BINDING_FRAME    = 0; // set0 b0
    constexpr uint32_t PRISM_VULKAN_BINDING_OBJECT   = 0; // set2 b0
    constexpr uint32_t PRISM_VULKAN_BINDING_MATERIAL = 0; // set3 b0

    // shadow/env/bloom 归 set1(RENDER_PASS)
    constexpr uint32_t PRISM_VULKAN_GLOBAL_TEXTURE_SET   = 1;
    constexpr uint32_t PRISM_VULKAN_GLOBAL_TEXTURE_BEGIN = 0;

    // 材质纹理归 set3(MATERIAL)
    constexpr uint32_t PRISM_VULKAN_TEXTURE_BEGIN_SET     = 3;
    constexpr uint32_t PRISM_VULKAN_TEXTURE_BEGIN_BINDING = 0;
}
