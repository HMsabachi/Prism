// PerObject UBO — binding=1

const int PRISM_MAX_BONES = 128;

layout(std140, binding = PRISM_BINDING_OBJECT) uniform PrismObject
{
    mat4 Prism_Model;
    mat4 Prism_PreviousModel;
    mat4 Prism_Bones[PRISM_MAX_BONES];
};
