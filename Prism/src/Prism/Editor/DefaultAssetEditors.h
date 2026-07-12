#pragma once

#include "AssetEditorPanel.h"
#include "Prism/Renderer/Mesh.h"

namespace Prism {

    class PRISM_API PhysicsMaterialEditor : public AssetEditor
    {
    public:
        PhysicsMaterialEditor();

        virtual void SetAsset(const Ref<Asset>& asset) override { m_Asset = (Ref<PhysicsMaterial>)asset; }

    private:
        virtual void Render() override;

    private:
        Ref<PhysicsMaterial> m_Asset;
    };

    class PRISM_API TextureViewer : public AssetEditor
    {
    public:
        TextureViewer();

        virtual void SetAsset(const Ref<Asset>& asset) override { m_Asset = (Ref<Texture2D>)asset; }

    private:
        virtual void Render() override;

    private:
        Ref<Texture2D> m_Asset;
    };

}
