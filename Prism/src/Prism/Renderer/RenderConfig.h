#pragma once

#include <string>
#include <glm/glm.hpp>
#include "Prism/Renderer/SceneEnvironment.h"

namespace Prism
{
    class Material;
    class TextureCube;

    struct Light
    {
        alignas(16) glm::vec3 Direction{ -0.5f, -1.0f, -0.5f };
        alignas(16) glm::vec3 Radiance{ 1.0f, 1.0f, 1.0f };
        alignas(4)  float Multiplier = 1.0f;
    };

    struct DirectionalLight
    {
        glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };
        float Multiplier = 0.0f;
        bool CastShadows = true;
        bool SoftShadows = true;
        float LightSize = 0.5f;
    };

    struct LightEnvironment
    {
        DirectionalLight DirectionalLights[4];
    };

    struct RenderConfig
    {
        Light SceneLight;
        float LightMultiplier = 0.3f;
        LightEnvironment LightEnvironment;
        Environment SceneEnvironment;
        float SceneEnvironmentIntensity = 1.0f;
        Ref<TextureCube> SkyboxTexture;
        Ref<Material> SkyboxMaterial;
        float SkyboxLod = 0.0f;
        bool ShadowsEnabled = true;
        float ShadowBias = 0.001f;
        float ShadowNormalBias = 0.1f;
        uint32_t CascadeCount = 4;
        float MaxShadowDistance = 200.0f;
        bool EnableBloom = false;
        float BloomThreshold = 1.5f;
    };
}
