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
		void RenderFileListView(int dirIndex);
		void RenderFileGridView(int dirIndex);
		void HandleDragDrop(RendererID icon, int dirIndex);
		void RenderDirectoriesListView(int dirIndex);
		void RenderDirectoriesGridView(int dirIndex);
		void RenderBreadCrumbs();
		void RenderSearch();
		void RenderBottom();

		ImGuiInputTextCallback SearchCallback(ImGuiInputTextCallbackData* data);

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

		std::string m_CurrentDirPath;
		std::string m_BaseDirPath;
		std::string m_PrevDirPath;
		std::string m_MovePath;

		std::string m_ForwardPath;
		std::string m_BackPath;

		int m_BasePathLen;
		int m_DirDataLen;

		bool m_IsDragging = false;
		bool m_DisplayListView = false;
		bool m_UpdateBreadCrumbs = true;
		bool m_ShowSearchBar = false;
		bool m_IsPathChanged = false;

		char m_InputBuffer[1024];

		std::vector<DirectoryInfo> m_CurrentDir;
		std::vector<DirectoryInfo> m_BaseProjectDir;

		ImGuiInputTextCallbackData m_Data;
		std::map<size_t, Ref<Texture2D>> m_AssetIconMap;
		AssetManager m_AssetManager;
	};

}
