#pragma once

#include "Prism/Asset/AssetManager.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/ImGui/ImGui.h"

#include <map>
#include <functional>

namespace Prism {

    template<typename T>
    struct SelectionStack
    {
    public:
        void Select(T item)
        {
            m_Selections.push_back(item);
        }

        void Deselect(T item)
        {
            for (auto it = m_Selections.begin(); it != m_Selections.end(); it++)
            {
                if (*it == item)
                {
                    m_Selections.erase(it);
                    break;
                }
            }
        }

        bool IsSelected(T item) const
        {
            for (auto selection : m_Selections)
            {
                if (selection == item)
                    return true;
            }

            return false;
        }

        void Clear()
        {
            m_Selections.clear();
        }

        size_t SelectionCount() const
        {
            return m_Selections.size();
        }

        T* GetSelectionData()
        {
            return m_Selections.data();
        }

    private:
        std::vector<T> m_Selections;
    };

    class PRISM_API AssetManagerPanel
    {
    public:
        AssetManagerPanel();
        void OnImGuiRender();

    private:
        void DrawDirectoryInfo(DirectoryInfo& dir);

        void RenderFileListView(Ref<Asset>& asset);
        void RenderFileGridView(Ref<Asset>& asset);
        void HandleDragDrop(RendererID icon, Ref<Asset>& asset);
        void RenderDirectoriesListView(DirectoryInfo& dirInfo);
        void RenderDirectoriesGridView(DirectoryInfo& dirInfo);
        void RenderBreadCrumbs();
        void RenderBottom();

        void HandleRenaming(const std::string& name, const std::function<void()>& callback);

        void UpdateCurrentDirectory(int dirIndex);

    private:
        Ref<Texture2D> m_FolderTex;
        Ref<Texture2D> m_FavoritesTex;
        Ref<Texture2D> m_FileTex;
        Ref<Texture2D> m_GoBackTex;
        Ref<Texture2D> m_ScriptTex;
        Ref<Texture2D> m_ResourceTex;
        Ref<Texture2D> m_SceneTex;

        Ref<Texture2D> m_BackbtnTex;
        Ref<Texture2D> m_FwrdbtnTex;
        Ref<Texture2D> m_FolderRightTex;
        Ref<Texture2D> m_TagsTex;
        Ref<Texture2D> m_SearchTex;
        Ref<Texture2D> m_GridView;
        Ref<Texture2D> m_ListView;

        std::string m_MovePath;

        int m_BaseDirIndex;
        int m_CurrentDirIndex;
        int m_PrevDirIndex;
        int m_NextDirIndex;

        bool m_IsDragging = false;
        bool m_DisplayListView = false;
        bool m_UpdateBreadCrumbs = true;
        bool m_DirectoryChanged = false;
        bool m_IsAnyItemHovered = false;

        char m_InputBuffer[1024];
        char m_RenameBuffer[512];

        DirectoryInfo m_CurrentDir;
        DirectoryInfo m_BaseProjectDir;
        std::vector<DirectoryInfo> m_CurrentDirChildren;
        std::vector<Ref<Asset>> m_CurrentDirAssets;

        std::vector<DirectoryInfo> m_BreadCrumbData;

        AssetHandle m_DraggedAssetId = 0;
        SelectionStack<AssetHandle> m_SelectedAssets;
        SelectionStack<int> m_SelectedDirectories;
        bool m_RenamingSelected = false;

        std::map<size_t, Ref<Texture2D>> m_AssetIconMap;
    };

}
