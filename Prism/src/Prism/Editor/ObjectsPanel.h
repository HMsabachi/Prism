#pragma once

#include "Prism/Renderer/Texture.h"
#include "Prism/Asset/AssetManager.h"

namespace Prism {

    class PRISM_API ObjectsPanel
    {
    public:
        ObjectsPanel();

        void OnImGuiRender();

    private:
        void DrawObject(const char* label, AssetHandle handle);

    private:
        Ref<Texture2D> m_CubeImage;
    };

}
