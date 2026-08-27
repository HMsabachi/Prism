#pragma once

#include "AssetSerializer.h"
#include "Prism/Core/Core.h"

#include <unordered_map>

namespace Prism {

    class PRISM_API AssetImporter
    {
    public:
        static void Init();
        static void Serialize(const Ref<Asset>& asset);
        static bool TryLoadData(Ref<Asset>& asset);

    private:
        static std::unordered_map<AssetType, Scope<AssetSerializer>> s_Serializers;
    };

}
