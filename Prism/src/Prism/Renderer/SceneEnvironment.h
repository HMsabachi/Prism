#pragma once

#include <string>
#include "Prism/Core/Core.h"

namespace Prism {

    class TextureCube;

    struct PRISM_API Environment
    {
        std::string FilePath;
        Ref<TextureCube> RadianceMap;
        Ref<TextureCube> IrradianceMap;

        Environment() = default;
        Environment(const std::string& filepath, const Ref<TextureCube>& radianceMap, const Ref<TextureCube>& irradianceMap)
            : FilePath(filepath), RadianceMap(radianceMap), IrradianceMap(irradianceMap) {}

        static Environment Load(const std::string& filepath);
    };

}
