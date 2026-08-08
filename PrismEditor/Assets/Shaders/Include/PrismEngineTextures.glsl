#ifndef PRISM_ENGINE_TEXTURES
#define PRISM_ENGINE_TEXTURES

// PrismEngineTextures.glsl

#if PRISM_BACKEND_OPENGL
#define PRISM_TEX(q) layout(binding = q)
#elif PRISM_BACKEND_VULKAN
#define PRISM_TEX(q) layout(set = PRISM_SET_RENDER_PASS, binding = q)
#endif

PRISM_TEX(PRISM_GEOMETRY_PASS_TEXTURE_BINDING) uniform sampler2DMS Prism_GeometryPassTexture;

PRISM_TEX(PRISM_ENV_RADIANCE_BINDING)          uniform samplerCube Prism_EnvRadianceTex;
PRISM_TEX(PRISM_ENV_IRRADIANCE_BINDING)        uniform samplerCube Prism_EnvIrradianceTex;
PRISM_TEX(PRISM_ENV_BRDF_LUT_BINDING)          uniform sampler2D   Prism_BRDFLUT;
PRISM_TEX(PRISM_BLOOM_TEXTURE_BINDING)         uniform sampler2D   Prism_BloomTexture;

#undef PRISM_TEX

#endif
