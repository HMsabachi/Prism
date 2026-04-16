#version 450 core
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;




const float PI = 3.141592;
const float TwoPI = 2.0 * PI;
const float Epsilon = 0.00001;
layout(binding = 0) uniform sampler2D u_EquirectangularTex;;
layout(binding = 1) uniform samplerCube u_InputCubeMap;;
layout(rgba16f, binding = 2) restrict writeonly uniform imageCube o_OutputCube;
uniform float u_Roughness;
float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}
vec2 sampleHammersley(uint i, uint N) {
    return vec2(float(i) / float(N), radicalInverse_VdC(i));
}
vec3 GetCubeMapDirection() {
    vec2 size = vec2(imageSize(o_OutputCube));
    vec2 st = vec2(gl_GlobalInvocationID.xy) / size;
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);
    vec3 ret;
    uint face = gl_GlobalInvocationID.z;
    if      (face == 0) ret = vec3(  1.0, uv.y, -uv.x);
    else if (face == 1) ret = vec3( -1.0, uv.y,  uv.x);
    else if (face == 2) ret = vec3( uv.x,  1.0, -uv.y);
    else if (face == 3) ret = vec3( uv.x, -1.0,  uv.y);
    else if (face == 4) ret = vec3( uv.x, uv.y,   1.0);
    else if (face == 5) ret = vec3(-uv.x, uv.y,  -1.0);
    return normalize(ret);
}
void computeBasisVectors(const vec3 N, out vec3 S, out vec3 T) {
    T = cross(N, vec3(0.0, 1.0, 0.0));
    T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));
    T = normalize(T);
    S = normalize(cross(N, T));
}
vec3 tangentToWorld(const vec3 v, const vec3 N, const vec3 S, const vec3 T) {
    return S * v.x + T * v.y + N * v.z;
}
vec3 sampleGGX(float u1, float u2, float roughness) {
    float alpha = roughness * roughness;
    float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha*alpha - 1.0) * u2));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
    float phi = TwoPI * u1;
    return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

void main() {
    vec3 cubeTC = GetCubeMapDirection();
    float phi = atan(cubeTC.z, cubeTC.x);
    float theta = acos(cubeTC.y);
    vec2 uv = vec2(phi / (TwoPI) + 0.5, theta / PI);
    vec4 color = texture(u_EquirectangularTex, uv);
    imageStore(o_OutputCube, ivec3(gl_GlobalInvocationID), color);
}

void CSEquirectToCube() {
    vec3 N = GetCubeMapDirection();
    vec3 S, T;
    computeBasisVectors(N, S, T);
    vec3 irradiance = vec3(0);
    const uint numSamples = 64 * 1024;
    for(uint i = 0; i < numSamples; i++) {
        vec2 u = sampleHammersley(i, numSamples);
        float u1p = sqrt(max(0.0, 1.0 - u.x * u.x));
        vec3 Li = tangentToWorld(vec3(cos(TwoPI*u.y) * u1p, sin(TwoPI*u.y) * u1p, u.x), N, S, T);
        float cosTheta = max(0.0, dot(Li, N));
        irradiance += 2.0 * textureLod(u_InputCubeMap, Li, 0).rgb * cosTheta;
    }
    imageStore(o_OutputCube, ivec3(gl_GlobalInvocationID), vec4(irradiance / float(numSamples), 1.0));
}

void CSEquirectToCube() {
    ivec2 outputSize = imageSize(o_OutputCube);
    if(gl_GlobalInvocationID.x >= outputSize.x || gl_GlobalInvocationID.y >= outputSize.y) return;
    vec3 N = GetCubeMapDirection();
    vec3 Lo = N;
    vec3 S, T;
    computeBasisVectors(N, S, T);
    vec3 color = vec3(0);
    float totalWeight = 0;
    const uint numSamples = 1024;
    vec2 inputSize = vec2(textureSize(u_InputCubeMap, 0));
    float wt = 4.0 * PI / (6.0 * inputSize.x * inputSize.y);
    for(uint i = 0; i < numSamples; i++) {
        vec2 u = sampleHammersley(i, numSamples);
        vec3 Lh = tangentToWorld(sampleGGX(u.x, u.y, u_Roughness), N, S, T);
        vec3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;
        float cosLi = dot(N, Li);
        if(cosLi > 0.0) {
            float cosLh = max(dot(N, Lh), 0.0);
            float alphaSq = pow(u_Roughness, 4.0);
            float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
            float pdf = (alphaSq / (PI * denom * denom)) * 0.25;
            float ws = 1.0 / (float(numSamples) * pdf);
            float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);
            color += textureLod(u_InputCubeMap, Li, mipLevel).rgb * cosLi;
            totalWeight += cosLi;
        }
    }
    imageStore(o_OutputCube, ivec3(gl_GlobalInvocationID), vec4(color / max(totalWeight, 0.001), 1.0));
}