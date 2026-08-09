#ifndef PASS_ENVIRONMENT
#define PASS_ENVIRONMENT

#include "Bindings.glsl"

// BloomBlurPass 纹理 IO 契约
// 输入:上一次 blur 结果 或 GeometryPass 的 bloom attachment
PRISM_PASS_TEX(0) uniform sampler2D Prism_BloomInputTexture;

#if PRISM_FRAGMENT_SHADER
layout(location = 0) out vec4 FragColor;
#endif

#endif
