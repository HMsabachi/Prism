#pragma once

#include <cstdint>

namespace Prism::Config
{
    //==================================================
    // 绑定层级
    // 全局层  帧级 UBO/SSBO(SetGlobal* 提交,Vulkan set 0)
    // Pass 层 pass 输入(RenderPass 自持)
    // 材质层  材质 UBO + 材质纹理(材质自持)
    //==================================================

    //==================================================
    // OpenGL 物理号 BASE 表
    // GL 有 4 个独立编号空间(UBO/SSBO/Texture/Image),各自从 0 起。
    // point = BASE + 槽位号
    //==================================================

    //---- UBO 空间(GL_UNIFORM_BUFFER, 84 个)----
    constexpr uint32_t GL_UBO_BASE_FRAME = 0;
    constexpr uint32_t GL_UBO_BASE_MATERIAL = 20;
    constexpr uint32_t GL_UBO_BASE_RENDER_PASS_NEW = 24;

    //---- SSBO 空间(GL_SHADER_STORAGE_BUFFER, 84 个)----
    constexpr uint32_t GL_SSBO_BASE_FRAME = 0;
    constexpr uint32_t GL_SSBO_BASE_MATERIAL = 20;
    constexpr uint32_t GL_SSBO_BASE_RENDER_PASS_NEW = 24;

    //---- Texture 空间----
    constexpr uint32_t GL_TEX_BASE_FRAME = 0;
    constexpr uint32_t GL_TEX_BASE_RENDER_PASS = 1;
    constexpr uint32_t GL_TEX_BASE_MATERIAL = 8;

    //---- Image 空间(image unit, 8 个)----
    constexpr uint32_t GL_IMG_BASE_FRAME = 0;
    constexpr uint32_t GL_IMG_BASE_RENDER_PASS = 1;
    constexpr uint32_t GL_IMG_BASE_MATERIAL = 4;

    //==================================================
    // Vulkan(直通, set/binding)
    //==================================================
    constexpr uint32_t PRISM_VULKAN_SET_GLOBAL = 0; // 全局层 
    constexpr uint32_t PRISM_VULKAN_SET_RENDER_PASS = 1; // Pass层
    constexpr uint32_t PRISM_VULKAN_SET_MATERIAL = 2; // 材质层 


}
