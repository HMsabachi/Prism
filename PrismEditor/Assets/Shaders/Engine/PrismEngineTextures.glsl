// PrismEngineTextures.glsl
layout(binding = PRISM_GEOMETRY_PASS_TEXTURE) uniform sampler2DMS Prism_GeometryPassTexture;

layout(binding = PRISM_ENV_RADIANCE)   uniform samplerCube Prism_EnvRadianceTex;
layout(binding = PRISM_ENV_IRRADIANCE) uniform samplerCube Prism_EnvIrradianceTex;
layout(binding = PRISM_ENV_BRDF_LUT)   uniform sampler2D   Prism_BRDFLUT;
