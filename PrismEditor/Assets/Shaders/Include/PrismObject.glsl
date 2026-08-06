// PerObject UBO — binding=1

#ifndef PRISM_OBJECT
#define PRISM_OBJECT

const int PRISM_MAX_BONES = 128;

#if PRISM_BACKEND_OPENGL
#define PRISM_OBJECT_LAYOUT layout(std140, binding = PRISM_BINDING_OBJECT)
#elif PRISM_BACKEND_VULKAN
#define PRISM_OBJECT_LAYOUT layout(std140, set = PRISM_SET_OBJECT, binding = PRISM_BINDING_OBJECT)
#endif

PRISM_OBJECT_LAYOUT uniform PrismObject
{
    mat4 Prism_Model;
    mat4 Prism_PreviousModel;
    vec4 Prism_ObjectReserved;       // .x = ShadowPassIndex
    mat4 Prism_Bones[PRISM_MAX_BONES];
};

#undef PRISM_OBJECT_LAYOUT

#endif
