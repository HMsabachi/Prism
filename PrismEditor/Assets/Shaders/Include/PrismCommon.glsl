// PrismCommon.glsl

vec4 PrismObjectToClipPos(vec3 pos)
{
    return Prism_ViewProjection * Prism_Model * vec4(pos, 1.0);
}

vec4 PrismObjectToWorldPos(vec3 pos)
{
    return Prism_Model * vec4(pos, 1.0);
}

vec4 PrismWorldToClipPos(vec3 pos)
{
    return Prism_ViewProjection * vec4(pos, 1.0);
}

vec3 PrismObjectToWorldNormal(vec3 normal)
{
    return normalize(mat3(Prism_Model) * normal);
}

vec3 PrismObjectToWorldDir(vec3 dir)
{
    return normalize(mat3(Prism_Model) * dir);
}

vec3 PrismWorldToObjectDir(vec3 dir)
{
    return normalize(mat3(inverse(Prism_Model)) * dir);
}

vec3 PrismWorldSpaceViewDir(vec3 worldPos)
{
    return normalize(Prism_CameraPosition - worldPos);
}

vec3 PrismObjectSpaceViewDir(vec3 objectPos)
{
    vec3 worldPos = (Prism_Model * vec4(objectPos, 1.0)).xyz;
    return normalize(Prism_CameraPosition - worldPos);
}

vec3 PrismWorldSpaceLightDir(vec3 worldPos)
{
    return normalize(-Prism_Lights[0].Direction);
}

vec3 PrismObjectSpaceLightDir(vec3 objectPos)
{
    return normalize(-Prism_Lights[0].Direction);
}


// 屏幕坐标

vec2 PrismComputeScreenPos(vec4 clipPos)
{
    return (clipPos.xy / clipPos.w) * 0.5 + 0.5;
}

vec2 PrismComputeGrabScreenPos(vec4 clipPos)
{
    return (clipPos.xy / clipPos.w) * 0.5 + 0.5;
}

// 副法线 (Binormal) / 切线空间

vec3 PrismCalcBinormal(vec3 normal, vec3 tangent, float tangentSign)
{
    return cross(normal, tangent) * tangentSign;
}

// 颜色空间

float PrismLinearToGamma(float c)
{
    vec3 threshold = vec3(c, c, c);
    vec3 linear    = 12.92 * threshold;
    vec3 gamma     = 1.055 * pow(threshold, vec3(1.0 / 2.4)) - 0.055;
    return mix(linear.x, gamma.x, step(0.0031308, c));
}

float PrismGammaToLinear(float c)
{
    vec3 threshold = vec3(c, c, c);
    vec3 linear    = threshold / 12.92;
    vec3 gamma     = pow((threshold + 0.055) / 1.055, vec3(2.4));
    return mix(linear.x, gamma.x, step(0.04045, c));
}

vec3 PrismLinearToGamma(vec3 c)
{
    return vec3(PrismLinearToGamma(c.r), PrismLinearToGamma(c.g), PrismLinearToGamma(c.b));
}

vec3 PrismGammaToLinear(vec3 c)
{
    return vec3(PrismGammaToLinear(c.r), PrismGammaToLinear(c.g), PrismGammaToLinear(c.b));
}

// 时间 (sin/cos 变体)

float PrismSinTime()
{
    return Prism_Time.y;
}

float PrismCosTime()
{
    return Prism_Time.z;
}

float PrismDeltaTime()
{
    return Prism_DeltaTime;
}

// 解包 / 重映射


vec3 PrismUnpackNormalMapIntensity(vec3 unpackedNormal, float intensity)
{
    unpackedNormal.xy *= intensity;
    unpackedNormal.z = sqrt(max(1.0 - dot(unpackedNormal.xy, unpackedNormal.xy), 0.0));
    return unpackedNormal;
}

float PrismLerpFloat(float a, float b, float t)
{
    return mix(a, b, t);
}

float PrismInverseLerp(float a, float b, float value)
{
    return (value - a) / (b - a);
}

float PrismRemap(float inMin, float inMax, float outMin, float outMax, float value)
{
    return mix(outMin, outMax, (value - inMin) / (inMax - inMin));
}
