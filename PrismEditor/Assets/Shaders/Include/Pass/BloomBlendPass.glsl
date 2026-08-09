#ifndef PASS_ENVIRONMENT
#define PASS_ENVIRONMENT

#include "Bindings.glsl"

// BloomBlendPass 纹理 IO 契约
// 输入:GeometryPass color + BloomBlur 结果
PRISM_PASS_TEX(0) uniform sampler2D Prism_GeometryPassTexture;
PRISM_PASS_TEX(1) uniform sampler2D Prism_BloomTexture;

#if PRISM_FRAGMENT_SHADER
layout(location = 0) out vec4 FragColor;
#endif

#endif
