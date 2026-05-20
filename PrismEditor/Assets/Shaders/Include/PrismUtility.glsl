const vec2 blurdf = vec2(0.4, 0.2);
vec4 PF_Fastblur(sampler2D tex, vec2 coord, float radius) {
    vec4 returns, norm = texture(tex, coord); if (radius <= 1.0) return norm;
    float lod = log2(radius); int ilod = int(lod); float n = ilod - lod + 1.0;
    vec2 blurdfr = vec2(blurdf.y, -blurdf.x), df = 1.0 / textureSize(tex, ilod), dfr = df * blurdfr;
    df *= blurdf; returns = n * (textureLod(tex, coord, ilod)
        + textureLod(tex, coord + df, ilod) + textureLod(tex, coord + dfr, ilod)
        + textureLod(tex, coord - df, ilod) + textureLod(tex, coord - dfr, ilod));
    df = 1.0 / textureSize(tex, ilod + 1), dfr = df * blurdfr, df *= blurdf;
    returns += (textureLod(tex, coord, ilod + 1)
        + textureLod(tex, coord + df, ilod + 1) + textureLod(tex, coord + dfr, ilod + 1)
        + textureLod(tex, coord - df, ilod + 1) + textureLod(tex, coord - dfr, ilod + 1));
    df = 1.0 / textureSize(tex, ilod + 2), dfr = df * blurdfr, df *= blurdf;
    returns += (1.0 - n) * (textureLod(tex, coord, ilod + 2)
        + textureLod(tex, coord + df, ilod + 2) + textureLod(tex, coord + dfr, ilod + 2)
        + textureLod(tex, coord - df, ilod + 2) + textureLod(tex, coord - dfr, ilod + 2));
    return mix(norm, returns * 0.1, clamp(2.0 * radius - 2.0, 0.0, 1.0));
}
// 旋转矩阵：输入弧度 a，返回旋转后的坐标
vec2 PF_Rotate(vec2 v, float a) {
    float s = sin(a);
    float c = cos(a);
    return mat2(c, -s, s, c) * v;
}

// 缩放坐标：从中心缩放
vec2 PF_Scale(vec2 v, float s) {
    return (v - 0.5) * s + 0.5;
}

// 经典的 2D 伪随机函数
float PF_Hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// 简单的数值噪声 (Value Noise)
float PF_Noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = PF_Hash21(i);
    float b = PF_Hash21(i + vec2(1.0, 0.0));
    float c = PF_Hash21(i + vec2(0.0, 1.0));
    float d = PF_Hash21(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f); // 平滑插值
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// HSV 转 RGB
vec3 PF_Hsv2Rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// 亮度计算 (Luminance)
float PF_GetLuminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

// 圆形 SDF
float PF_SdCircle(vec2 p, float r) {
    return length(p) - r;
}

// 矩形 SDF
float PF_SdBox(vec2 p, vec2 b) {
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

// 抗锯齿绘图包装函数
// dist: SDF距离, thickness: 线条粗细, blur: 平滑度
float PF_DrawShape(float dist, float thickness, float blur) {
    return smoothstep(blur, -blur, dist - thickness);
}

// 适配分辨率
vec2 PF_FixUV(vec2 fragCoord, vec2 resolution) {
    return (2.0 * fragCoord - resolution.xy) / min(resolution.y, resolution.x);
}