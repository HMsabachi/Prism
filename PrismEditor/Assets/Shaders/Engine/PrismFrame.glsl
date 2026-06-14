// PerFrame UBO — binding=0
struct Prism_Light
{
    vec3 Direction;
    vec3 Radiance;
    float Multiplier;
};

const int PRISM_MAX_LIGHTS   = 1;
const int PRISM_MAX_CASCADES = 4;

layout(std140, binding = PRISM_BINDING_FRAME) uniform PrismFrame
{
    mat4 Prism_ViewProjection;
    mat4 Prism_InverseViewProjection;
    mat4 Prism_View;
    mat4 Prism_Projection;

    vec4 Prism_Time;
    vec3 Prism_CameraPosition;

    float Prism_DeltaTime;
    float Prism_AspectRatio;
    vec2  Prism_Resolution;

    Prism_Light Prism_Lights[PRISM_MAX_LIGHTS];

    mat4 Prism_ShadowMatrices[PRISM_MAX_CASCADES];
    vec4 Prism_CascadeSplits;
    vec4 Prism_ShadowParams;
};
