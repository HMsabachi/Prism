#include "prpch.h"
#include "AssetManagerPanel.h"
#include "Prism/Core/Application.h"
#include "Prism/Utilities/DragDropData.h"

#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)
PR_WARNING_DISABLE(4267)
PR_WARNING_DISABLE(4244)

namespace Prism {

	AssetManagerPanel::AssetManagerPanel()
	{
		AssetTypes::Init();

		m_FolderTex = Texture2D::Create("assets/editor/folder.png");
		m_FavoritesTex = Texture2D::Create("assets/editor/favourites.png");
		m_GoBackTex = Texture2D::Create("assets/editor/back.png");
		m_ScriptTex = Texture2D::Create("assets/editor/script.png");
		m_ResourceTex = Texture2D::Create("assets/editor/resource.png");
		m_SceneTex = Texture2D::Create("assets/editor/scene.png");

		m_AssetIconMaps[-1] = Texture2D::Create("assets/editor/file.png");
		m_AssetIconMaps[AssetTypes::GetAssetTypeID("fbx")] = Texture2D::Create("assets/editor/fbx.png");
		m_AssetIconMaps[AssetTypes::GetAssetTypeID("obj")] = Texture2D::Create("assets/editor/obj.png");
		m_AssetIconMaps[AssetTypes::GetAssetTypeID("wav")] = Texture2D::Create("assets/editor/wav.png");
		m_AssetIconMaps[AssetTypes::GetAssetTypeID("cs")] = Texture2D::Create("assets/editor/csc.png");
		m_AssetIconMaps[AssetTypes::GetAssetTypeID("png")] = Texture2D::Create("assets/editor/png.png");
		m_AssetIconMaps[AssetTypes::GetAssetTypeID("blend")] = Texture2D::Create("assets/editor/blend.png");
		m_AssetIconMaps[AssetTypes::GetAssetTypeID("psc")] = Texture2D::Create("assets/editor/prism.png");

		m_BackbtnTex = Texture2D::Create("assets/editor/btn_back.png");
		m_FwrdbtnTex = Texture2D::Create("assets/editor/btn_fwrd.png");
		m_FolderRightTex = Texture2D::Create("assets/editor/folder_hierarchy.png");
		m_SearchTex = Texture2D::Create("assets/editor/search.png");
		m_TagsTex = Texture2D::Create("assets/editor/tags.png");
		m_GridView = Texture2D::Create("assets/editor/grid.png");
		m_ListView = Texture2D::Create("assets/editor/list.png");

		m_BaseDirPath = "assets";
		m_CurrentDirPath = m_BaseDirPath;
		m_PrevDirPath = m_CurrentDirPath;
		m_BaseProjectDir = m_AssetManager.GetFileSystemContents();
		m_CurrentDir = m_BaseProjectDir;
		m_BasePathLen = strlen(m_BaseDirPath.c_str());
		m_DirDataLen = 0;

		memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
	}

	void AssetManagerPanel::OnImGuiRender()
	{
		ImGui::Begin("Project", NULL, ImGuiWindowFlags_None);
		{
			UI::BeginPropertyGrid();
			ImGui::SetColumnOffset(1, 240);

			// There happens to be recursive tree unfolding issue which doesn't show nested directories/files
			ImGui::BeginChild("##folders_common");
			{
				if (ImGui::CollapsingHeader("res://", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (ImGui::TreeNode("Contents"))
					{
						for (int i = 0; i < m_BaseProjectDir.size(); i++)
						{
							if (ImGui::TreeNode(m_BaseProjectDir[i].Filename.c_str()))
							{
								auto dirData = m_AssetManager.GetDirectoryContents(m_BaseProjectDir[i].AbsolutePath);
								for (int i = 0; i < dirData.size(); i++)
								{
									if (!dirData[i].IsFile)
									{
										if (ImGui::TreeNode(dirData[i].Filename.c_str()))
											ImGui::TreePop();
									}
									else
									{
										std::string parentDir = m_AssetManager.GetParentPath(dirData[i].AbsolutePath);
										ImGui::Indent();
										ImGui::Selectable(dirData[i].Filename.c_str(), false);
										ImGui::Unindent();
									}
								}
								ImGui::TreePop();
							}

							if (m_IsDragging && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
							{
								m_MovePath = m_BaseProjectDir[i].AbsolutePath.c_str();
							}
						}
						ImGui::TreePop();
					}

					if (ImGui::IsMouseDown(1))
					{
						//ImGui::OpenPopup("window");
					}
				}

				ImGui::EndChild();
			}

			if (ImGui::BeginDragDropTarget())
			{
				auto data = ImGui::AcceptDragDropPayload("selectable", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
				if (data)
				{
					std::string file = (char*)data->Data;
					if (m_AssetManager.MoveFile(file, m_MovePath))
					{
						PR_CORE_INFO("Moved File: " + file + " to " + m_MovePath);
						m_CurrentDir = m_AssetManager.GetDirectoryContents(m_CurrentDirPath);
					}
					m_IsDragging = false;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::NextColumn();

			ImGui::BeginChild("##directory_structure", ImVec2(ImGui::GetColumnWidth() - 12, ImGui::GetWindowHeight() - 50));
			{
				RenderBreadCrumbs();
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(ImGui::GetColumnWidth() - 350, 0));
				ImGui::SameLine();
				RenderSearch();
				ImGui::EndChild();

				ImGui::BeginChild("Scrolling");

				if (!m_DisplayListView)
					ImGui::Columns(10, nullptr, false);

				for (int i = 0; i < m_CurrentDir.size(); i++)
				{
					if (m_CurrentDir.size() > 0)
					{
						if (!m_DisplayListView)
							m_CurrentDir[i].IsFile ? RenderFileGridView(i) : RenderDirectoriesGridView(i);
						else
							m_CurrentDir[i].IsFile ? RenderFileListView(i) : RenderDirectoriesListView(i);

						ImGui::NextColumn();
					}
				}

				ImGui::EndChild();
				ImGui::EndChild();
			}

			if (ImGui::BeginDragDropTarget())
			{
				auto data = ImGui::AcceptDragDropPayload("selectable", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
				if (data)
				{
					std::string a = (char*)data->Data;
					if (m_AssetManager.MoveFile(a, m_MovePath))
					{
						PR_CORE_INFO("Moved File: " + a + " to " + m_MovePath);
					}
					m_IsDragging = false;
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Create"))
				{
					if (ImGui::MenuItem("Import New Asset", "Ctrl + O"))
					{
						std::string filename = Application::Get().OpenFile("");
						m_AssetManager.ProcessAsset(filename);
					}

					if (ImGui::MenuItem("Refresh", "Ctrl + R"))
					{
						auto data = m_AssetManager.GetFileSystemContents();
						for (int i = 0; i < data.size(); i++)
						{
							PR_CORE_INFO(data[i].Filename);
						}
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			UI::EndPropertyGrid();
			ImGui::End();
		}
	}

	void AssetManagerPanel::RenderFileListView(int dirIndex)
	{
		ImGui::PushID(dirIndex);
		size_t fileID = AssetTypes::GetAssetTypeID(m_CurrentDir[dirIndex].FileType);
		RendererID iconRef = m_AssetIconMaps[fileID]->GetRendererID();
		ImGui::Image((ImTextureID)iconRef, ImVec2(20, 20));

		ImGui::SameLine();

		if (ImGui::Selectable(m_CurrentDir[dirIndex].Filename.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
		{
			if (ImGui::IsMouseDoubleClicked(0))
				m_AssetManager.HandleAsset(m_CurrentDir[dirIndex].AbsolutePath);
		}

		HandleDragDrop(iconRef, dirIndex);
		ImGui::PopID();
	}

	void AssetManagerPanel::RenderFileGridView(int dirIndex)
	{
		ImGui::PushID(dirIndex);
		ImGui::BeginGroup();

		size_t fileID = AssetTypes::GetAssetTypeID(m_CurrentDir[dirIndex].FileType);
		RendererID iconRef = m_AssetIconMaps[fileID]->GetRendererID();
		float columnWidth = ImGui::GetColumnWidth();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
		ImGui::ImageButton((ImTextureID)iconRef, { columnWidth - 10.0F, columnWidth - 10.0F });
		ImGui::PopStyleColor(3);

		HandleDragDrop(iconRef, dirIndex);

		std::string newFileName = m_AssetManager.StripExtras(m_CurrentDir[dirIndex].Filename);
		ImGui::TextWrapped(newFileName.c_str());
		ImGui::EndGroup();
		ImGui::PopID();
	}

	void AssetManagerPanel::HandleDragDrop(RendererID icon, int dirIndex)
	{
		if (ImGui::BeginDragDropSource())
		{
			ImGui::Image((ImTextureID)icon, ImVec2(20, 20));
			ImGui::SameLine();
			ImGui::Text(m_CurrentDir[dirIndex].Filename.c_str());

			int size = sizeof(const char*) + strlen(m_CurrentDir[dirIndex].AbsolutePath.c_str());
			ImGui::SetDragDropPayload("selectable", m_CurrentDir[dirIndex].AbsolutePath.c_str(), size);

			AssetType assetType = AssetTypes::GetAssetTypeFromExtension(m_CurrentDir[dirIndex].FileType);

			if (assetType == AssetType::Mesh)
			{
				DragDropData d("Mesh", m_CurrentDir[dirIndex].AbsolutePath.c_str(), m_CurrentDir[dirIndex].Filename.c_str());
				ImGui::SetDragDropPayload("scene_entity_assetsP", &d, sizeof(d));
			}

			if (assetType == AssetType::Scene)
			{
				DragDropData d("PrismScene", m_CurrentDir[dirIndex].AbsolutePath.c_str(), m_CurrentDir[dirIndex].Filename.c_str());
				ImGui::SetDragDropPayload("scene_entity_assetsP", &d, sizeof(d));
			}

			m_IsDragging = true;
			ImGui::EndDragDropSource();
		}
	}

	void AssetManagerPanel::RenderDirectoriesListView(int dirIndex)
	{
		ImGui::PushID(dirIndex);
		ImGui::Image((ImTextureID)m_FolderTex->GetRendererID(), ImVec2(20, 20));
		ImGui::SameLine();

		if (ImGui::Selectable(m_CurrentDir[dirIndex].Filename.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
		{
			if (ImGui::IsMouseDoubleClicked(0))
			{
				m_PrevDirPath = m_CurrentDir[dirIndex].AbsolutePath;
				m_CurrentDirPath = m_CurrentDir[dirIndex].AbsolutePath;
				m_CurrentDir = m_AssetManager.GetDirectoryContents(m_CurrentDir[dirIndex].AbsolutePath);
			}
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
		{
			ImGui::Image((ImTextureID)m_FolderTex->GetRendererID(), ImVec2(20, 20));
			ImGui::SameLine();
			ImGui::Text(m_CurrentDir[dirIndex].Filename.c_str());
			int size = sizeof(const char*) + strlen(m_CurrentDir[dirIndex].AbsolutePath.c_str());
			ImGui::SetDragDropPayload("selectable", m_CurrentDir[dirIndex].AbsolutePath.c_str(), size);
			m_IsDragging = true;
			ImGui::EndDragDropSource();
		}
		ImGui::PopID();
	}

	void AssetManagerPanel::RenderDirectoriesGridView(int dirIndex)
	{
		ImGui::PushID(dirIndex);
		ImGui::BeginGroup();

		float columnWidth = ImGui::GetColumnWidth();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
		ImGui::ImageButton((ImTextureID)m_FolderTex->GetRendererID(), { columnWidth - 10.0F, columnWidth - 10.0F });
		ImGui::PopStyleColor(3);

		if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
		{
			m_PrevDirPath = m_CurrentDir[dirIndex].AbsolutePath;
			m_CurrentDirPath = m_CurrentDir[dirIndex].AbsolutePath;
			m_CurrentDir = m_AssetManager.GetDirectoryContents(m_CurrentDir[dirIndex].AbsolutePath);
			m_IsPathChanged = true;
			m_DirDataLen = m_CurrentDir.size();
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
		{
			ImGui::Image((ImTextureID)m_FolderTex->GetRendererID(), ImVec2(20, 20));
			ImGui::SameLine();
			ImGui::Text(m_CurrentDir[dirIndex].Filename.c_str());
			int size = sizeof(const char*) + strlen(m_CurrentDir[dirIndex].AbsolutePath.c_str());
			ImGui::SetDragDropPayload("selectable", m_CurrentDir[dirIndex].AbsolutePath.c_str(), size);
			m_IsDragging = true;
			ImGui::EndDragDropSource();
		}

		if (!m_IsPathChanged)
		{
			auto fname = m_CurrentDir[dirIndex].Filename;
			auto newFname = m_AssetManager.StripExtras(fname);
			ImGui::TextWrapped(newFname.c_str());
		}
		else
		{
			for (int i = 0; i < m_DirDataLen; i++)
			{
				auto fname = m_CurrentDir[i].Filename;
				auto newFname = m_AssetManager.StripExtras(fname);
				ImGui::TextWrapped(newFname.c_str());
				m_IsPathChanged = false;
				m_DirDataLen = 0;
			}
		}

		ImGui::EndGroup();
		ImGui::PopID();
	}

	void AssetManagerPanel::RenderBreadCrumbs()
	{
		ImGui::BeginChild("##directory_breadcrumbs", ImVec2(ImGui::GetColumnWidth() - 100, 30));
		{
			RendererID viewTexture = m_DisplayListView ? m_ListView->GetRendererID() : m_GridView->GetRendererID();
			if (ImGui::ImageButton((ImTextureID)viewTexture, ImVec2(20, 18)))
			{
				m_DisplayListView = !m_DisplayListView;
			}

			ImGui::SameLine();

			if (ImGui::ImageButton((ImTextureID)m_SearchTex->GetRendererID(), ImVec2(20, 18)))
				m_ShowSearchBar = !m_ShowSearchBar;

			ImGui::SameLine();

			if (m_ShowSearchBar)
			{
				ImGui::SameLine();
				ImGui::PushItemWidth(200);

				// TODO: Search function is currently broken
				if (ImGui::InputTextWithHint("", "Search...", m_InputBuffer, 100, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					m_CurrentDir = m_AssetManager.SearchFiles(m_InputBuffer, m_CurrentDirPath);
				}

				ImGui::PopItemWidth();
				ImGui::SameLine();
			}

			if (ImGui::ImageButton((ImTextureID)m_BackbtnTex->GetRendererID(), ImVec2(20, 18)))
			{
				if (strlen(m_CurrentDirPath.c_str()) == m_BasePathLen) return;
				m_ForwardPath = m_CurrentDirPath;
				m_BackPath = m_AssetManager.GetParentPath(m_CurrentDirPath);
				m_CurrentDir = m_AssetManager.GetDirectoryContents(m_BackPath);
				m_CurrentDirPath = m_BackPath;
			}

			ImGui::SameLine();

			if (ImGui::ImageButton((ImTextureID)m_FwrdbtnTex->GetRendererID(), ImVec2(20, 18)))
			{
				m_CurrentDir = m_AssetManager.GetDirectoryContents(m_ForwardPath);
				m_CurrentDirPath = m_ForwardPath;
			}

			ImGui::SameLine();

			auto data = m_AssetManager.GetDirectories(m_CurrentDirPath);

			for (int i = 0; i < data.size(); i++)
			{
				if (data[i] != m_BaseDirPath)
				{
					ImGui::Image((ImTextureID)m_FolderRightTex->GetRendererID(), ImVec2(22, 23));
				}

				ImGui::SameLine();

				/* Multiply the size of the folder name with 7(magic number) to set the size of selectable widget properly */
				int size = strlen(data[i].c_str()) * 7;

				if (ImGui::Selectable(data[i].c_str(), false, 0, ImVec2(size, 22)))
				{
					/* Increament 1 to the existing index value to fully iterate the array */
					int index = i + 1;
					std::string path = "";

					/* Use the below loop to build a path from the selected folder from the breadcrumb to navigate to */
					for (int e = 0; e < index; e++)
					{
						path += data[e] + "/\\";
					}

					m_CurrentDir = m_AssetManager.GetDirectoryContents(path);
					m_CurrentDirPath = path;
				}
				ImGui::SameLine();
			}

			ImGui::SameLine();

			ImGui::Dummy(ImVec2(ImGui::GetColumnWidth() - 400, 0));

			ImGui::SameLine();
		}
	}

	void AssetManagerPanel::RenderSearch()
	{
		/*ImGui::BeginChild("##search_menu", ImVec2(320, 30));
		{
			char buff[100] = { 0 };
			ImGui::Image((ImTextureID)m_searchTex->GetRendererID(), ImVec2(22, 22));
			ImGui::SameLine();
			ImGui::InputTextWithHint(inputText, inputHint, buff, 100);
			ImGui::SameLine();
			ImGui::ImageButton((ImTextureID)m_favoritesTex->GetRendererID(), ImVec2(19, 19));
			ImGui::SameLine();
			ImGui::ImageButton((ImTextureID)m_TagsTex->GetRendererID(), ImVec2(19, 19));
		}
		ImGui::EndChild();*/
	}

	void AssetManagerPanel::RenderBottom()
	{
		/* Will be used for object select indication. Ex. 3 folders 1 file selected */
		ImGui::BeginChild("##nav", ImVec2(ImGui::GetColumnWidth() - 12, 23));
		{
			ImGui::EndChild();
		}
	}
}
