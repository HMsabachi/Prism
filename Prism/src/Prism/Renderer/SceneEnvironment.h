#pragma once

#include "Prism/Asset/Asset.h"

namespace Prism {

    class TextureCube;

    class PRISM_API Environment : public Asset
    {
    public:
        Ref<TextureCube> RadianceMap;
        Ref<TextureCube> IrradianceMap;

        Environment() = default;
        Environment(const Ref<TextureCube>& radianceMap, const Ref<TextureCube>& irradianceMap)
            : RadianceMap(radianceMap), IrradianceMap(irradianceMap) {}
    };

}
