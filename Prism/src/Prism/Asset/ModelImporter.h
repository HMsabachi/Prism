#pragma once

#include <vector>
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Material.h"

namespace Prism
{
    struct ModelImportResult
    {
        Ref<Mesh> Mesh;
        std::vector<Ref<Material>> Materials;
    };

    class ModelImporter
    {
    public:
        static ModelImportResult Import(const std::string& filepath);
    };
}
