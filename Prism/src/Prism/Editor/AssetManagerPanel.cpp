#include "prpch.h"
#include "AssetManagerPanel.h"
#include "AssetEditorPanel.h"
#include "Prism/Core/Application.h"
#include "Prism/Core/Input.h"
#include "Prism/Utilities/FileSystem.h"

#include <filesystem>

#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)
PR_WARNING_DISABLE(4267)
PR_WARNING_DISABLE(4244)

namespace Prism {

    AssetManagerPanel::AssetManagerPanel()
    {
        AssetManager::SetAssetChangeCallback([&]()
        {
            UpdateCurrentDirectory(m_CurrentDirIndex);
        });

        m_FolderTex = Texture2D::Create("assets/editor/folder.png");
        m_FavoritesTex = Texture2D::Create("assets/editor/favourites.png");
        m_GoBackTex = Texture2D::Create("assets/editor/back.png");
        m_ScriptTex = Texture2D::Create("assets/editor/script.png");
        m_ResourceTex = Texture2D::Create("assets/editor/resource.png");
        m_SceneTex = Texture2D::Create("assets/editor/scene.png");

        m_AssetIconMap[-1] = Texture2D::Create("assets/editor/file.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("hdr")] = Texture2D::Create("assets/editor/file.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("fbx")] = Texture2D::Create("assets/editor/fbx.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("obj")] = Texture2D::Create("assets/editor/obj.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("wav")] = Texture2D::Create("assets/editor/wav.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("cs")] = Texture2D::Create("assets/editor/csc.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("py")] = Texture2D::Create("assets/editor/pyc.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("png")] = Texture2D::Create("assets/editor/png.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("blend")] = Texture2D::Create("assets/editor/blend.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("psc")] = Texture2D::Create("assets/editor/prism.png");
        m_AssetIconMap[AssetTypes::GetAssetTypeID("Shader")] = Texture2D::Create("assets/editor/shader.png");
        //m_AssetIconMap[AssetTypes::GetAssetTypeID("glsl")] = m_AssetIconMap[AssetTypes::GetAssetTypeID("Shader")];

        m_BackbtnTex = Texture2D::Create("assets/editor/btn_back.png");
        m_FwrdbtnTex = Texture2D::Create("assets/editor/btn_fwrd.png");
        m_FolderRightTex = Texture2D::Create("assets/editor/folder_hierarchy.png");
        m_SearchTex = Texture2D::Create("assets/editor/search.png");
        m_TagsTex = Texture2D::Create("assets/editor/tags.png");
        m_GridView = Texture2D::Create("assets/editor/grid.png");
        m_ListView = Texture2D::Create("assets/editor/list.png");

        m_BaseDirIndex = 0;
        m_BaseProjectDir = AssetManager::GetDirectoryInfo(m_BaseDirIndex);
        UpdateCurrentDirectory(m_BaseDirIndex);

        memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
    }

    void AssetManagerPanel::DrawDirectoryInfo(DirectoryInfo& dir)
    {
        if (ImGui::TreeNode(dir.DirectoryName.c_str()))
        {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                UpdateCurrentDirectory(dir.DirectoryIndex);

            for (int j = 0; j < dir.ChildrenIndices.size(); j++)
            {
                DrawDirectoryInfo(AssetManager::GetDirectoryInfo(dir.ChildrenIndices[j]));
            }

            ImGui::TreePop();
        }

        if (m_IsDragging && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        {
            m_MovePath = dir.FilePath;
        }
    }

    void AssetManagerPanel::OnImGuiRender()
    {
        ImGui::Begin("Project", NULL, ImGuiWindowFlags_None);
        {
            UI::BeginPropertyGrid();
            ImGui::SetColumnOffset(1, 240);

            ImGui::BeginChild("##folders_common");
            {
                if (ImGui::CollapsingHeader("Content", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (int i = 0; i < m_BaseProjectDir.ChildrenIndices.size(); i++)
                    {
                        DrawDirectoryInfo(AssetManager::GetDirectoryInfo(m_BaseProjectDir.ChildrenIndices[i]));
                    }
                }
            }
            ImGui::EndChild();

            if (ImGui::BeginDragDropTarget())
            {
                auto data = ImGui::AcceptDragDropPayload("directory_payload", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                if (data)
                {
                    std::string file = (char*)data->Data;
                    /*if (AssetManager::MoveFile(file, m_MovePath))
                    {
                        PR_CORE_INFO("Moved File: " + file + " to " + m_MovePath);
                        UpdateCurrentDirectory(m_CurrentDirIndex);
                    }*/
                    m_IsDragging = false;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::NextColumn();

            ImGui::BeginChild("##directory_structure", ImVec2(ImGui::GetColumnWidth() - 12, ImGui::GetWindowHeight() - 50));
            {
                ImGui::BeginChild("##directory_breadcrumbs", ImVec2(ImGui::GetColumnWidth() - 100, 30));
                RenderBreadCrumbs();
                ImGui::EndChild();

                ImGui::BeginChild("Scrolling");

                if (!m_DisplayListView)
                    ImGui::Columns(10, nullptr, false);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2F, 0.205F, 0.21F, 0.25F));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3F, 0.3F, 0.3F, 0.3F));

                if (Input::IsKeyPressed(KeyCode::Escape) || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_IsAnyItemHovered))
                {
                    m_SelectedAssets.Clear();
                    m_SelectedDirectories.Clear();

                    m_RenamingSelected = false;
                }

                m_IsAnyItemHovered = false;

                if (ImGui::BeginPopupContextWindow(0, 1))
                {
                    if (ImGui::BeginMenu("Create"))
                    {
                        if (ImGui::MenuItem("Folder"))
                        {
                            bool created = std::filesystem::create_directory(m_CurrentDir.FilePath + "/New Folder");

                            if (created)
                            {
                                UpdateCurrentDirectory(m_CurrentDirIndex);
                                auto& createdDirectory = AssetManager::GetDirectoryInfo(m_CurrentDir.ChildrenIndices[m_CurrentDir.ChildrenIndices.size() - 1]);
                                m_SelectedDirectories.Select(createdDirectory.DirectoryIndex);
                                memset(m_RenameBuffer, 0, 512);
                                memcpy(m_RenameBuffer, createdDirectory.DirectoryName.c_str(), createdDirectory.DirectoryName.size());
                                m_RenamingSelected = true;
                            }
                        }

                        if (ImGui::MenuItem("Scene"))
                            PR_CORE_INFO("Creating Scene...");

                        if (ImGui::MenuItem("Script"))
                            PR_CORE_INFO("Creating Script...");

                        if (ImGui::MenuItem("Prefab"))
                            PR_CORE_INFO("Creating Prefab...");

                        if (ImGui::MenuItem("Physics Material"))
                        {
                            AssetManager::CreateAsset<PhysicsMaterial>("New Physics Material.ppm", AssetType::PhysicsMat, m_CurrentDirIndex, 0.6F, 0.6F, 0.0F);
                            UpdateCurrentDirectory(m_CurrentDirIndex);
                        }

                        ImGui::EndMenu();
                    }

                    if (ImGui::MenuItem("Import"))
                    {
                    }

                    if (ImGui::MenuItem("Refresh"))
                    {
                        UpdateCurrentDirectory(m_CurrentDirIndex);
                    }

                    ImGui::EndPopup();
                }

                for (DirectoryInfo& dir : m_CurrentDirChildren)
                {
                    if (m_DisplayListView)
                        RenderDirectoriesListView(dir);
                    else
                        RenderDirectoriesGridView(dir);

                    ImGui::NextColumn();
                }

                for (Ref<Asset>& asset : m_CurrentDirAssets)
                {
                    if (m_DisplayListView)
                        RenderFileListView(asset);
                    else
                        RenderFileGridView(asset);

                    ImGui::NextColumn();
                }

                if (m_IsDragging && !ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.1F))
                {
                    m_IsDragging = false;
                    m_DraggedAssetId = 0;
                }

                ImGui::PopStyleColor(3);

                ImGui::EndChild();
            }
            ImGui::EndChild();

            if (ImGui::BeginDragDropTarget())
            {
                auto data = ImGui::AcceptDragDropPayload("directory_payload", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                if (data)
                {
                    std::string a = (char*)data->Data;
                    /*if (AssetManager::MoveFile(a, m_MovePath))
                    {
                        UpdateCurrentDirectory(m_CurrentDirIndex);
                    }*/

                    m_IsDragging = false;
                }
                ImGui::EndDragDropTarget();
            }

            UI::EndPropertyGrid();
        }
        ImGui::End();
    }

    void AssetManagerPanel::RenderDirectoriesListView(DirectoryInfo& dirInfo)
    {
        ImGui::Image((ImTextureID)m_FolderTex->GetRendererID(), ImVec2(30, 30));
        ImGui::SameLine();

        if (ImGui::Selectable(dirInfo.DirectoryName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 30)))
        {
            if (ImGui::IsMouseDoubleClicked(0))
            {
                m_PrevDirIndex = m_CurrentDirIndex;
                UpdateCurrentDirectory(dirInfo.DirectoryIndex);
            }
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
        {
            ImGui::Image((ImTextureID)m_FolderTex->GetRendererID(), ImVec2(20, 20));
            ImGui::SameLine();
            ImGui::Text(dirInfo.DirectoryName.c_str());
            ImGui::SetDragDropPayload("directory_payload", &dirInfo.DirectoryIndex, sizeof(int));
            m_IsDragging = true;
            ImGui::EndDragDropSource();
        }
    }

    void AssetManagerPanel::RenderDirectoriesGridView(DirectoryInfo& dirInfo)
    {
        ImGui::PushID(dirInfo.FilePath.c_str());
        ImGui::BeginGroup();
        float columnWidth = ImGui::GetColumnWidth();

        if (m_SelectedDirectories.IsSelected(dirInfo.DirectoryIndex))
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25F, 0.25F, 0.25F, 0.75F));

        ImGui::ImageButton((ImTextureID)m_FolderTex->GetRendererID(), { columnWidth - 15.0F, columnWidth - 15.0F });

        if (m_SelectedDirectories.IsSelected(dirInfo.DirectoryIndex))
            ImGui::PopStyleColor();

        if (ImGui::IsItemHovered())
        {
            m_IsAnyItemHovered = true;

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_PrevDirIndex = m_CurrentDirIndex;
                UpdateCurrentDirectory(dirInfo.DirectoryIndex);
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (!Input::IsKeyPressed(KeyCode::LeftControl))
                {
                    m_SelectedDirectories.Clear();
                    m_SelectedAssets.Clear();
                }

                if (m_SelectedDirectories.IsSelected(dirInfo.DirectoryIndex))
                    m_SelectedDirectories.Deselect(dirInfo.DirectoryIndex);
                else
                    m_SelectedDirectories.Select(dirInfo.DirectoryIndex);
            }
        }

        bool shouldOpenDeleteModal = false;

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Rename"))
            {
                m_SelectedDirectories.Select(dirInfo.DirectoryIndex);
                memset(m_RenameBuffer, 0, 512);
                memcpy(m_RenameBuffer, dirInfo.DirectoryName.c_str(), dirInfo.DirectoryName.size());
                m_RenamingSelected = true;
            }

            if (ImGui::MenuItem("Delete"))
            {
                shouldOpenDeleteModal = true;
            }

            ImGui::EndPopup();
        }

        if (shouldOpenDeleteModal)
        {
            ImGui::OpenPopup("Delete Folder");
            shouldOpenDeleteModal = false;
        }

        if (ImGui::BeginPopupModal("Delete Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Are you sure you want to delete %s and everything within it?", dirInfo.DirectoryName.c_str());
            float columnWidth = ImGui::GetContentRegionAvail().x / 4;

            ImGui::Columns(4, 0, false);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::SetColumnWidth(1, columnWidth);
            ImGui::SetColumnWidth(2, columnWidth);
            ImGui::SetColumnWidth(3, columnWidth);
            ImGui::NextColumn();
            if (ImGui::Button("Yes", ImVec2(columnWidth, 0)))
            {
                bool deleted = FileSystem::DeleteFile(dirInfo.FilePath);
                if (deleted)
                {
                    AssetManager::RemoveDirectory(dirInfo.DirectoryIndex);
                    UpdateCurrentDirectory(m_CurrentDirIndex);
                }

                ImGui::CloseCurrentPopup();
            }

            ImGui::NextColumn();
            ImGui::SetItemDefaultFocus();
            if (ImGui::Button("No", ImVec2(columnWidth, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::NextColumn();
            ImGui::EndPopup();
        }

        ImVec2 prevCursorPos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(prevCursorPos.x + 10.0F, prevCursorPos.y - 21.0F));
        ImGui::SetNextItemWidth(columnWidth - 15.0F);

        if (!m_SelectedDirectories.IsSelected(dirInfo.DirectoryIndex) || !m_RenamingSelected)
            ImGui::TextWrapped(dirInfo.DirectoryName.c_str());

        ImGui::SetCursorPos(prevCursorPos);

        if (m_SelectedDirectories.IsSelected(dirInfo.DirectoryIndex))
            HandleRenaming(dirInfo.DirectoryName, [&]() { AssetManager::Rename(dirInfo.DirectoryIndex, m_RenameBuffer); });

        ImGui::EndGroup();
        ImGui::PopID();
    }

    void AssetManagerPanel::RenderFileListView(Ref<Asset>& asset)
    {
        size_t fileID = AssetTypes::GetAssetTypeID(asset->Extension);
        fileID = m_AssetIconMap.find(fileID) != m_AssetIconMap.end() ? fileID : -1;
        RendererID iconRef = m_AssetIconMap[fileID]->GetRendererID();
        ImGui::Image((ImTextureID)iconRef, ImVec2(30, 30));

        ImGui::SameLine();

        ImGui::Selectable(asset->FileName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 30));

        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                AssetEditorPanel::OpenEditor(asset);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (!Input::IsKeyPressed(KeyCode::LeftControl))
                {
                    m_SelectedDirectories.Clear();
                    m_SelectedAssets.Clear();
                }

                m_SelectedAssets.Select(asset->Handle);
            }
        }

        HandleDragDrop(iconRef, asset);
    }

    void AssetManagerPanel::RenderFileGridView(Ref<Asset>& asset)
    {
        ImGui::PushID(&asset->Handle);
        ImGui::BeginGroup();

        size_t fileID = AssetTypes::GetAssetTypeID(asset->Extension);
        fileID = m_AssetIconMap.find(fileID) != m_AssetIconMap.end() ? fileID : -1;
        RendererID iconRef = m_AssetIconMap[fileID]->GetRendererID();
        float columnWidth = ImGui::GetColumnWidth();

        if (m_SelectedAssets.IsSelected(asset->Handle))
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25F, 0.25F, 0.25F, 0.75F));

        ImGui::ImageButton((ImTextureID)iconRef, { columnWidth - 15.0F, columnWidth - 15.0F });

        if (m_SelectedAssets.IsSelected(asset->Handle))
            ImGui::PopStyleColor();

        HandleDragDrop(iconRef, asset);

        if (ImGui::IsItemHovered())
        {
            m_IsAnyItemHovered = true;

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                AssetEditorPanel::OpenEditor(asset);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_IsDragging)
            {
                if (!Input::IsKeyPressed(KeyCode::LeftControl))
                {
                    m_SelectedDirectories.Clear();
                    m_SelectedAssets.Clear();
                }

                if (m_SelectedAssets.IsSelected(asset->Handle))
                    m_SelectedAssets.Deselect(asset->Handle);
                else
                    m_SelectedAssets.Select(asset->Handle);
            }
        }

        bool shouldOpenDeleteModal = false;

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Rename"))
            {
                m_SelectedAssets.Select(asset->Handle);
                memset(m_RenameBuffer, 0, 512);
                memcpy(m_RenameBuffer, asset->FileName.c_str(), asset->FileName.size());
                m_RenamingSelected = true;
            }

            if (ImGui::MenuItem("Delete"))
            {
                shouldOpenDeleteModal = true;
            }

            ImGui::EndPopup();
        }

        if (shouldOpenDeleteModal)
        {
            ImGui::OpenPopup("Delete File");
            shouldOpenDeleteModal = false;
        }

        bool deleted = false;
        if (ImGui::BeginPopupModal("Delete File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Are you sure you want to delete %s?", asset->FileName.c_str());
            float columnWidth = ImGui::GetContentRegionAvail().x / 4;

            ImGui::Columns(4, 0, false);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::SetColumnWidth(1, columnWidth);
            ImGui::SetColumnWidth(2, columnWidth);
            ImGui::SetColumnWidth(3, columnWidth);
            ImGui::NextColumn();
            if (ImGui::Button("Yes", ImVec2(columnWidth, 0)))
            {
                std::string filepath = asset->FilePath;
                AssetHandle assetHandle = asset->Handle;
                deleted = FileSystem::DeleteFile(filepath);
                if (deleted)
                {
                    FileSystem::DeleteFile(filepath + ".meta");
                    AssetManager::RemoveAsset(assetHandle);
                    UpdateCurrentDirectory(m_CurrentDirIndex);
                }

                ImGui::CloseCurrentPopup();
            }

            ImGui::NextColumn();
            ImGui::SetItemDefaultFocus();
            if (ImGui::Button("No", ImVec2(columnWidth, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::NextColumn();
            ImGui::EndPopup();
        }

        if (!deleted)
        {
            ImGui::SetNextItemWidth(columnWidth - 15.0F);

            if (!m_SelectedAssets.IsSelected(asset->Handle) || !m_RenamingSelected)
                ImGui::TextWrapped(asset->FileName.c_str());

            if (m_SelectedAssets.IsSelected(asset->Handle))
                HandleRenaming(asset->FileName, [&]() { AssetManager::Rename(asset, m_RenameBuffer); });
        }

        ImGui::EndGroup();
        ImGui::PopID();
    }

    void AssetManagerPanel::HandleDragDrop(RendererID icon, Ref<Asset>& asset)
    {
        if (!m_SelectedAssets.IsSelected(asset->Handle) || m_IsDragging)
            return;

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped) && ImGui::IsItemClicked(ImGuiMouseButton_Left))
            m_IsDragging = true;

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::Image((ImTextureID)icon, ImVec2(20, 20));
            ImGui::SameLine();
            ImGui::Text(asset->FileName.c_str());
            ImGui::SetDragDropPayload("asset_payload", m_SelectedAssets.GetSelectionData(), m_SelectedAssets.SelectionCount() * sizeof(AssetHandle));
            m_IsDragging = true;
            ImGui::EndDragDropSource();
        }
    }

    void AssetManagerPanel::RenderBreadCrumbs()
    {
        RendererID viewTexture = m_DisplayListView ? m_ListView->GetRendererID() : m_GridView->GetRendererID();
        if (ImGui::ImageButton((ImTextureID)viewTexture, ImVec2(20, 18)))
        {
            m_DisplayListView = !m_DisplayListView;
        }

        ImGui::SameLine();

        if (ImGui::ImageButton((ImTextureID)m_BackbtnTex->GetRendererID(), ImVec2(20, 18)))
        {
            if (m_CurrentDirIndex == m_BaseDirIndex) return;
            m_NextDirIndex = m_CurrentDirIndex;
            m_PrevDirIndex = AssetManager::GetDirectoryInfo(m_CurrentDirIndex).ParentIndex;
            UpdateCurrentDirectory(m_PrevDirIndex);
        }

        ImGui::SameLine();

        if (ImGui::ImageButton((ImTextureID)m_FwrdbtnTex->GetRendererID(), ImVec2(20, 18)))
        {
            UpdateCurrentDirectory(m_NextDirIndex);
        }

        ImGui::SameLine();

        {
            ImGui::PushItemWidth(200);

            if (ImGui::InputTextWithHint("##search", "Search...", m_InputBuffer, 100))
            {
                SearchResults results = AssetManager::SearchFiles(m_InputBuffer, m_CurrentDir.FilePath);

                if (strlen(m_InputBuffer) == 0)
                {
                    UpdateCurrentDirectory(m_CurrentDirIndex);
                }
                else
                {
                    m_CurrentDirChildren = results.Directories;
                    m_CurrentDirAssets = results.Assets;
                }
            }

            ImGui::PopItemWidth();
        }

        ImGui::SameLine();

        if (m_UpdateBreadCrumbs)
        {
            m_BreadCrumbData.clear();

            int currentDirIndex = m_CurrentDirIndex;
            while (currentDirIndex != -1)
            {
                DirectoryInfo& dirInfo = AssetManager::GetDirectoryInfo(currentDirIndex);
                m_BreadCrumbData.push_back(dirInfo);
                currentDirIndex = dirInfo.ParentIndex;
            }

            std::reverse(m_BreadCrumbData.begin(), m_BreadCrumbData.end());

            m_UpdateBreadCrumbs = false;
        }

        for (int i = 0; i < m_BreadCrumbData.size(); i++)
        {
            if (m_BreadCrumbData[i].DirectoryName != "assets")
                ImGui::Text("/");

            ImGui::SameLine();

            int size = strlen(m_BreadCrumbData[i].DirectoryName.c_str()) * 7;

            if (ImGui::Selectable(m_BreadCrumbData[i].DirectoryName.c_str(), false, 0, ImVec2(size, 22)))
            {
                UpdateCurrentDirectory(m_BreadCrumbData[i].DirectoryIndex);
            }

            ImGui::SameLine();
        }

        ImGui::SameLine();

        ImGui::Dummy(ImVec2(ImGui::GetColumnWidth() - 400, 0));
        ImGui::SameLine();
    }

    void AssetManagerPanel::RenderBottom()
    {
        ImGui::BeginChild("##nav", ImVec2(ImGui::GetColumnWidth() - 12, 23));
        {
            ImGui::EndChild();
        }
    }

    void AssetManagerPanel::HandleRenaming(const std::string& name, const std::function<void()>& callback)
    {
        size_t totalSelection = m_SelectedAssets.SelectionCount() + m_SelectedDirectories.SelectionCount();
        if (totalSelection > 1)
            return;

        if (!m_RenamingSelected && Input::IsKeyPressed(KeyCode::F2))
        {
            memset(m_RenameBuffer, 0, 512);
            memcpy(m_RenameBuffer, name.c_str(), name.size());
            m_RenamingSelected = true;
        }

        if (m_RenamingSelected)
        {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##rename_dummy", m_RenameBuffer, 512, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                PR_CORE_INFO("Renaming to {0}", m_RenameBuffer);
                callback();
                m_RenamingSelected = false;
                m_SelectedAssets.Clear();
                m_SelectedDirectories.Clear();
                UpdateCurrentDirectory(m_CurrentDirIndex);
            }
        }
    }

    void AssetManagerPanel::UpdateCurrentDirectory(int dirIndex)
    {
        if (m_CurrentDirIndex != dirIndex)
            m_UpdateBreadCrumbs = true;

        m_CurrentDirChildren.clear();
        m_CurrentDirAssets.clear();

        m_CurrentDirIndex = dirIndex;
        m_CurrentDir = AssetManager::GetDirectoryInfo(m_CurrentDirIndex);

        for (int childIndex : m_CurrentDir.ChildrenIndices)
            m_CurrentDirChildren.push_back(AssetManager::GetDirectoryInfo(childIndex));

        m_CurrentDirAssets = AssetManager::GetAssetsInDirectory(m_CurrentDirIndex);
    }

}
