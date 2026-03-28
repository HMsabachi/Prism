#pragma once
#include <glm/glm.hpp>

namespace Prism {

    // 与 GLSL PrismGlobals 完全对应的 std140 结构体
    // 注意对齐规则：vec3 之后要 padding 到 16 字节
    struct PrismGlobalsUBO
    {
        glm::mat4 ViewProjection{ 1.0f };
        glm::mat4 Model{ 1.0f };
        glm::mat4 View{ 1.0f };
        glm::mat4 Projection{ 1.0f };

        glm::vec4 Time{ 0.0f };           // x/y/z/w 如上
        glm::vec3 CameraPosition{ 0.0f };
        float     _padding0{ 0.0f };      // 手动 padding，确保 vec3 后对齐

        float DeltaTime{ 0.0f };
        float AspectRatio{ 1.0f };
        glm::vec2 Resolution{ 1280.0f, 720.0f };

        // 预留空间，方便以后扩展（保持 256 字节对齐更好）
        float _padding1[48]{ 0.0f };
    };

    class PRISM_API GlobalUniforms
    {
    public:
        static void Init();
        virtual ~GlobalUniforms() = default;

        static void UpdateGlobalUniform(const PrismGlobalsUBO& globalUniforms)
        {s_Instance->UpdateGlobalUniformBuffer(globalUniforms); }
        
    private:
        virtual void UpdateGlobalUniformBuffer(const PrismGlobalsUBO& globalUniforms) const = 0;
    private:
        static GlobalUniforms* s_Instance;
    };

}