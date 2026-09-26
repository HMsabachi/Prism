#ifndef PASS_ENVIRONMENT
#define PASS_ENVIRONMENT

#include "Bindings.glsl"

// CompositePass 纹理 IO 契约
PRISM_PASS_TEXTURE(0) uniform sampler2D Prism_GeometryPassTexture;
PRISM_PASS_TEXTURE(1) uniform sampler2D Prism_ObjectIDTexture;
PRISM_PASS_TEXTURE(2) uniform sampler2D Prism_BloomTexture;
PRISM_PASS_TEXTURE(3) uniform sampler2D Prism_SSRTexture;

#if PRISM_FRAGMENT_SHADER
layout(location = 0) out vec4 FragColor;
#endif

#endif
