#ifndef PASS_ENVIRONMENT
#define PASS_ENVIRONMENT

#include "Bindings.glsl"

// CompositePass 纹理 IO 契约
PRISM_PASS_TEX(0) uniform sampler2DMS Prism_GeometryPassTexture;

#if PRISM_FRAGMENT_SHADER
layout(location = 0) out vec4 FragColor;
#endif

#endif
