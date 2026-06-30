#pragma once

#include "Prism/Utilities/AssetManager.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/ImGui/ImGui.h"

#include <map>

namespace Prism {

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

        char m_InputBuffer[1024];

        DirectoryInfo m_CurrentDir;
        DirectoryInfo m_BaseProjectDir;
        std::vector<DirectoryInfo> m_CurrentDirChildren;
        std::vector<Ref<Asset>> m_CurrentDirAssets;

        std::vector<DirectoryInfo> m_BreadCrumbData;

        AssetHandle m_DraggedAssetId = 0;

        std::map<size_t, Ref<Texture2D>> m_AssetIconMap;
    };

}
