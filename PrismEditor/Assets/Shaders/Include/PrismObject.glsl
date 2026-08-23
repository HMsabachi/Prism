// PerObject SSBO - set=2 (TRANSFORMS)

#ifndef PRISM_OBJECT
#define PRISM_OBJECT

const int PRISM_MAX_BONES = 128;

struct PrismObjectData
{
    mat4 ObjectToWorld;
    mat4 PreviousModel;
    vec4 Reserved;
    int  AnimationOffset;
};

PRISM_GOLBAL_STORAGE_BUFFER(1) buffer PrismObjects
{
    PrismObjectData Prism_Objects[];
};

PRISM_GOLBAL_STORAGE_BUFFER(2) buffer PrismAnimation
{
    mat4 Prism_AllBones[];
};

// drawIndex
#if PRISM_BACKEND_OPENGL
layout(location = 0) uniform int Prism_DrawIndex;
#elif PRISM_BACKEND_VULKAN
layout(push_constant) uniform PrismDrawIndexPC { int Prism_DrawIndex; };
#endif

#define Prism_ObjectToWorld (Prism_Objects[Prism_DrawIndex].ObjectToWorld)
#define Prism_PreviousModel (Prism_Objects[Prism_DrawIndex].PreviousModel)
#define Prism_ObjectReserved (Prism_Objects[Prism_DrawIndex].Reserved)
#define Prism_GetBoneMatrix(i) (Prism_AllBones[Prism_Objects[Prism_DrawIndex].AnimationOffset + (i)])


#endif
