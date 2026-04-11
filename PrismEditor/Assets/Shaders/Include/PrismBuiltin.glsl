// ==================== Prism 引擎全局 Uniform Block ====================
layout(std140, binding = 0) uniform PrismGlobals
{
    mat4 Prism_ViewProjection;   // VP 矩阵（View * Projection）
    mat4 Prism_InverseViewProjection; // VP 矩阵的逆矩阵
    mat4 Prism_Model_legacy;            // 当前物体的 Model 矩阵（每个 DrawCall 更新）
    mat4 Prism_View;             // View 矩阵
    mat4 Prism_Projection;       // Projection 矩阵（可选）
    
    vec4 Prism_Time;             // x: time/20, y: time, z: time*2, w: time*3（常用动画）
    vec3 Prism_CameraPosition;   // 相机世界位置（用于 Fresnel、反射等）
    
    float Prism_DeltaTime;       // 帧间隔（秒）
    float Prism_AspectRatio;     // 屏幕宽高比
    vec2  Prism_Resolution;      // 屏幕分辨率 (width, height)
    
};
uniform mat4 Prism_Model; // 当前物体的 Model 矩阵（每个 DrawCall 更新）