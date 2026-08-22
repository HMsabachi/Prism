#ifndef PRISM_BINDINGS
#define PRISM_BINDINGS

#if PRISM_BACKEND_OPENGL

// OpenGL: 4 个独立编号空间(UBO/SSBO/Texture/Image),layout(binding=物理号)
// UBO 空间(GL_UNIFORM_BUFFER)
#   define PRISM_SET_FRAME        0
#   define PRISM_SET_RENDER_PASS  1
#   define PRISM_SET_OBJECT       2
#   define PRISM_SET_MATERIAL     3


#   define PRISM_GOLBAL_TEXTURE(slot) layout(binding = (0 + (slot)))
#   define PRISM_GOLBAL_STORAGE_BUFFER(slot) layout(binding = (0 + (slot)))
#   define PRISM_GOLBAL_UNIFORM_BUFFER(slot) layout(binding = (0 + (slot)))

#   define PRISM_PASS_TEXTURE(slot) layout(binding = (1 + (slot)))
#   define PRISM_PASS_STORAGE_BUFFER(slot) layout(binding = (24 + (slot)))
#   define PRISM_PASS_UNIFORM_BUFFER(slot) layout(binding = (24 + (slot)))

#   define PRISM_BINDING_TEXTURE 8

#elif PRISM_BACKEND_VULKAN

// Vulkan: set/binding 直通
#   define PRISM_SET_FRAME        0
#   define PRISM_SET_RENDER_PASS  1
#   define PRISM_SET_OBJECT       2
#   define PRISM_SET_MATERIAL     3


#   define PRISM_GOLBAL_TEXTURE(slot) layout(set = PRISM_SET_FRAME, binding = (slot))
#   define PRISM_GOLBAL_STORAGE_BUFFER(slot) layout(set = PRISM_SET_FRAME, binding = (slot))
#   define PRISM_GOLBAL_UNIFORM_BUFFER(slot) layout(set = PRISM_SET_FRAME, binding = (slot))

#   define PRISM_PASS_TEXTURE(slot) layout(set = PRISM_SET_RENDER_PASS, binding = (slot))
#   define PRISM_PASS_STORAGE_BUFFER(slot) layout(set = PRISM_SET_RENDER_PASS, binding = (slot))
#   define PRISM_PASS_UNIFORM_BUFFER(slot) layout(set = PRISM_SET_RENDER_PASS, binding = (slot))

// Texture(set3 MATERIAL)
#   define PRISM_BINDING_TEXTURE 0

#endif

#endif
