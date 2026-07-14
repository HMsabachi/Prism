#include "prpch.h"
#include "AssetEditorPanel.h"
#include "DefaultAssetEditors.h"
#include "Prism/Asset/AssetManager.h"

namespace Prism {

    AssetEditor::AssetEditor(const char* title)
        : m_Title(title), m_MinSize(200, 400), m_MaxSize(2000, 2000)
    {
    }

    void AssetEditor::OnImGuiRender()
    {
        if (!m_IsOpen)
            return;

        bool was_open = m_IsOpen;
        ImGui::SetNextWindowSizeConstraints(m_MinSize, m_MaxSize);
        ImGui::Begin(m_Title, &m_IsOpen, m_Flags);
        Render();
        ImGui::End();

        if (was_open && !m_IsOpen)
            OnClose();
    }

    void AssetEditor::SetOpen(bool isOpen)
    {
        m_IsOpen = isOpen;
        if (!m_IsOpen)
            OnClose();
    }

    void AssetEditor::SetMinSize(uint32_t width, uint32_t height)
    {
        if (width <= 0) width = 200;
        if (height <= 0) height = 400;

        m_MinSize = ImVec2((float)width, (float)height);
    }

    void AssetEditor::SetMaxSize(uint32_t width, uint32_t height)
    {
        if (width <= 0) width = 2000;
        if (height <= 0) height = 2000;
        if (width <= (uint32_t)m_MinSize.x) width = (uint32_t)m_MinSize.x * 2;
        if (height <= (uint32_t)m_MinSize.y) height = (uint32_t)m_MinSize.y * 2;

        m_MaxSize = ImVec2((float)width, (float)height);
    }

    void AssetEditorPanel::RegisterDefaultEditors()
    {
        RegisterEditor<TextureViewer>(AssetType::Texture);
        RegisterEditor<PhysicsMaterialEditor>(AssetType::PhysicsMat);
    }

    void AssetEditorPanel::UnregisterAllEditors()
    {
        s_Editors.clear();
    }

    void AssetEditorPanel::OnImGuiRender()
    {
        for (auto& kv : s_Editors)
            kv.second->OnImGuiRender();
    }

    void AssetEditorPanel::OpenEditor(const Ref<Asset>& asset)
    {
        if (s_Editors.find(asset->Type) == s_Editors.end())
        {
            PR_CORE_WARN("No editor registered for {0} assets", asset->Extension);
            return;
        }

        s_Editors[asset->Type]->SetOpen(true);
        s_Editors[asset->Type]->SetAsset(AssetManager::GetAsset<Asset>(asset->Handle));
    }

    std::unordered_map<AssetType, Scope<AssetEditor>> AssetEditorPanel::s_Editors;

}
