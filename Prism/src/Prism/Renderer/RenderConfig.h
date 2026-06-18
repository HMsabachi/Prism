#pragma once

#include <string>
#include <glm/glm.hpp>
#include "Prism/Renderer/Texture.h"

namespace Prism
{
    class Material;

    struct Light
    {
        alignas(16) glm::vec3 Direction{ -0.5f, -1.0f, -0.5f };
        alignas(16) glm::vec3 Radiance{ 1.0f, 1.0f, 1.0f };
        alignas(4)  float Multiplier = 1.0f;
    };

    struct PRISM_API Environment
    {
        std::string FilePath;
        Ref<TextureCube> RadianceMap;
        Ref<TextureCube> IrradianceMap;
        static Environment Load(const std::string& filepath);
    };

    struct RenderConfig
    {
        Light SceneLight;
        float LightMultiplier = 0.3f;
        Environment SceneEnvironment;
        Ref<TextureCube> SkyboxTexture;
        Ref<Material> SkyboxMaterial;
        float SkyboxLod = 0.0f;
        bool ShadowsEnabled = true;
        float ShadowBias = 0.001f;
        float ShadowNormalBias = 0.1f;
        uint32_t CascadeCount = 4;
    };
}
