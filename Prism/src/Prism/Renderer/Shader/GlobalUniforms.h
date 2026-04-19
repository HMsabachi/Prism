#pragma once
#include <glm/glm.hpp>
#include "Prism/Scene/Scene.h"

namespace Prism {

    struct PrismGlobalsUBO
    {
        glm::mat4 ViewProjection{ 1.0f };
		glm::mat4 InverseViewProjection{ 1.0f };
        glm::mat4 Model{ 1.0f };
        glm::mat4 View{ 1.0f };
        glm::mat4 Projection{ 1.0f };

        glm::vec4 Time{ 0.0f };           
        glm::vec3 CameraPosition{ 0.0f };
        float     _padding0{ 0.0f };     
        float DeltaTime{ 0.0f };
        float AspectRatio{ 1.0f };
        glm::vec2 Resolution{ 1280.0f, 720.0f };

        Light Lights[MAX_LIGHTS];

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