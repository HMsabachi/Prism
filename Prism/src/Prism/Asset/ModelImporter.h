#pragma once

#include <vector>
#include "Prism/Core/Core.h"

namespace Prism
{
    class Mesh;
    class Material;

    struct ModelImportResult
    {
        Ref<Mesh> Mesh;
        std::vector<Ref<Material>> Materials;
    };

    class PRISM_API ModelImporter
    {
    public:
        static ModelImportResult Import(const std::string& filepath);
    };
}
