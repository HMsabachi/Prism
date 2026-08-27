#pragma once

#include "Prism/Asset/AssetManager.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/ImGui/ImGui.h"

#include <map>
#include <functional>

#define MAX_INPUT_BUFFER_LENGTH 128

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

    class PRISM_API ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();
        void OnImGuiRender();

    private:
        void DrawDirectoryInfo(AssetHandle directory);

        void RenderAsset(Ref<Asset>& asset);
        void HandleDragDrop(Ref<Image2D> icon, Ref<Asset>& asset);
        void RenderBreadCrumbs();

        void HandleRenaming(Ref<Asset>& asset);

        void UpdateCurrentDirectory(AssetHandle directoryHandle);

    private:
        Ref<Texture2D> m_FileTex;

        Ref<Texture2D> m_BackbtnTex;
        Ref<Texture2D> m_FwrdbtnTex;
        Ref<Texture2D> m_FolderRightTex;
        Ref<Texture2D> m_SearchTex;

        std::string m_MovePath;

        AssetHandle m_CurrentDirHandle;
        AssetHandle m_BaseDirectoryHandle;
        AssetHandle m_PrevDirHandle;
        AssetHandle m_NextDirHandle;

        bool m_IsDragging = false;
        bool m_UpdateBreadCrumbs = true;
        bool m_IsAnyItemHovered = false;
        bool m_UpdateDirectoryNextFrame = false;

        char m_RenameBuffer[MAX_INPUT_BUFFER_LENGTH];
        char m_SearchBuffer[MAX_INPUT_BUFFER_LENGTH];

        Ref<Directory> m_CurrentDirectory;
        Ref<Directory> m_BaseDirectory;
        std::vector<Ref<Asset>> m_CurrentDirFiles;
        std::vector<Ref<Asset>> m_CurrentDirFolders;

        std::vector<Ref<Directory>> m_BreadCrumbData;

        AssetHandle m_DraggedAssetId = 0;
        SelectionStack<AssetHandle> m_SelectedAssets;
        bool m_RenamingSelected = false;

        std::map<std::string, Ref<Texture2D>> m_AssetIconMap;
    };

}
