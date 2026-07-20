#include "prpch.h"
#include "DefaultAssetEditors.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Asset/AssetImporter.h"

namespace Prism {

    PhysicsMaterialEditor::PhysicsMaterialEditor()
        : AssetEditor("Edit Physics Material") {}

    void PhysicsMaterialEditor::OnClose()
    {
        AssetImporter::Serialize(m_Asset);
        m_Asset = nullptr;
    }

    void PhysicsMaterialEditor::Render()
    {
        if (!m_Asset)
            SetOpen(false);

        UI::BeginPropertyGrid();
        UI::Property("Static Friction", m_Asset->StaticFriction);
        UI::Property("Dynamic Friction", m_Asset->DynamicFriction);
        UI::Property("Bounciness", m_Asset->Bounciness);
        UI::EndPropertyGrid();
    }

    TextureViewer::TextureViewer()
        : AssetEditor("Edit Texture")
    {
        SetMinSize(200, 600);
        SetMaxSize(500, 1000);
    }

    void TextureViewer::OnClose()
    {
        m_Asset = nullptr;
    }

    void TextureViewer::Render()
    {
        if (!m_Asset)
            SetOpen(false);

        float textureWidth = (float)m_Asset->GetWidth();
        float textureHeight = (float)m_Asset->GetHeight();
        float bitsPerPixel = (float)Texture::GetBPP(m_Asset->GetFormat());
        float imageSize = ImGui::GetWindowWidth() - 40;
        imageSize = glm::min(imageSize, 500.0F);

        ImGui::SetCursorPosX(20);
        UI::Image(m_Asset->GetImage(), { imageSize, imageSize });

        UI::BeginPropertyGrid();
        UI::Property("Width", textureWidth, 0.1F, 0.0F, 0.0F, UI::PropertyFlag::None, true);
        UI::Property("Height", textureHeight, 0.1F, 0.0F, 0.0F, UI::PropertyFlag::None, true);
        UI::Property("Bits", bitsPerPixel, 0.1F, 0.0F, 0.0F, UI::PropertyFlag::None, true);
        UI::EndPropertyGrid();
    }

}
