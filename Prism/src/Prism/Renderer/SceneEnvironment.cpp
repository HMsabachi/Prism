#include "prpch.h"
#include "SceneEnvironment.h"
#include "Prism/Renderer/RenderPipeline.h"

namespace Prism {

    Environment Environment::Load(const std::string& filepath)
    {
        auto [radiance, irradiance] = RenderPipeline::CreateEnvironmentMap(filepath);
        return { filepath, radiance, irradiance };
    }

}
