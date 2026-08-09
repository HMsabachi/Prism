#ifndef PRISM_BINDINGS
#define PRISM_BINDINGS

#if PRISM_BACKEND_OPENGL

// OpenGL: 4 个独立编号空间(UBO/SSBO/Texture/Image),layout(binding=物理号)
// UBO 空间(GL_UNIFORM_BUFFER)
#   define PRISM_SET_FRAME        0
#   define PRISM_SET_RENDER_PASS  1
#   define PRISM_SET_OBJECT       2
#   define PRISM_SET_MATERIAL     3

#   define PRISM_BINDING_FRAME    0
#   define PRISM_BINDING_OBJECT   16
#   define PRISM_BINDING_MATERIAL 20

#   define PRISM_PASS_TEX(slot) layout(binding = (1 + (slot)))

#   define PRISM_BINDING_TEXTURE 12

#elif PRISM_BACKEND_VULKAN

// Vulkan: set/binding 直通
#   define PRISM_SET_FRAME        0
#   define PRISM_SET_RENDER_PASS  1
#   define PRISM_SET_OBJECT       2
#   define PRISM_SET_MATERIAL     3

#   define PRISM_BINDING_FRAME    0
#   define PRISM_BINDING_OBJECT   0
#   define PRISM_BINDING_MATERIAL 0

// Texture(set1 RENDER_PASS, per-pass 复用)
#   define PRISM_PASS_TEX(slot) layout(set = PRISM_SET_RENDER_PASS, binding = (slot))

// Texture(set3 MATERIAL)
#   define PRISM_BINDING_TEXTURE 0

#endif

#endif
