// PerObject SSBO - set=2 (TRANSFORMS)

#ifndef PRISM_OBJECT
#define PRISM_OBJECT

const int PRISM_MAX_BONES = 128;          // 单 mesh 骨骼上限(局部索引范围)

#if PRISM_BACKEND_OPENGL
#define PRISM_OBJECT_LAYOUT layout(std430, binding = PRISM_BINDING_OBJECT)
#define PRISM_ANIM_LAYOUT layout(std430, binding = PRISM_BINDING_ANIMATION)
#elif PRISM_BACKEND_VULKAN
#define PRISM_OBJECT_LAYOUT layout(std430, set = PRISM_SET_OBJECT, binding = PRISM_BINDING_OBJECT)
#define PRISM_ANIM_LAYOUT layout(std430, set = PRISM_SET_OBJECT, binding = PRISM_BINDING_ANIMATION)
#endif

struct PrismObjectData
{
    mat4 ObjectToWorld;
    mat4 PreviousModel;
    vec4 Reserved;        // .x = ShadowPassIndex, .y = ObjectID
    int  AnimationOffset; // 骨骼大数组起始索引, -1 = 无动画
};

PRISM_OBJECT_LAYOUT buffer PrismObjects
{
    PrismObjectData Prism_Objects[];
};

#ifdef SKINNED
PRISM_ANIM_LAYOUT buffer PrismAnimation
{
    mat4 Prism_AllBones[];
};
#endif

// drawIndex 传输通道
#if PRISM_BACKEND_OPENGL
layout(location = 0) uniform int Prism_DrawIndex;
#elif PRISM_BACKEND_VULKAN
layout(push_constant) uniform PrismDrawIndexPC { int Prism_DrawIndex; };
#endif

#define Prism_ObjectToWorld  (Prism_Objects[Prism_DrawIndex].ObjectToWorld)
#define Prism_PreviousModel  (Prism_Objects[Prism_DrawIndex].PreviousModel)
#define Prism_ObjectReserved (Prism_Objects[Prism_DrawIndex].Reserved)
#define PRISM_BONE(i)        (Prism_AllBones[Prism_Objects[Prism_DrawIndex].AnimationOffset + (i)])

#undef PRISM_OBJECT_LAYOUT
#undef PRISM_ANIM_LAYOUT

#endif
