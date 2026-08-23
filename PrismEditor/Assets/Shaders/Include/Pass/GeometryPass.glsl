#ifndef PASS_ENVIRONMENT
#define PASS_ENVIRONMENT

#include "Bindings.glsl"

// GeometryPass 纹理 IO 契约
// 输入纹理(per-pass slot,从 0 数)
PRISM_PASS_TEXTURE(0) uniform sampler2D Prism_ShadowMap0;
PRISM_PASS_TEXTURE(1) uniform sampler2D Prism_ShadowMap1;
PRISM_PASS_TEXTURE(2) uniform sampler2D Prism_ShadowMap2;
PRISM_PASS_TEXTURE(3) uniform sampler2D Prism_ShadowMap3;
PRISM_PASS_TEXTURE(4) uniform samplerCube Prism_EnvRadianceTex;
PRISM_PASS_TEXTURE(5) uniform samplerCube Prism_EnvIrradianceTex;
PRISM_PASS_TEXTURE(6) uniform sampler2D Prism_BRDFLUT;

#if PRISM_FRAGMENT_SHADER
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BloomColor;
layout(location = 2) out vec4 NormalColor;
#endif

#endif
