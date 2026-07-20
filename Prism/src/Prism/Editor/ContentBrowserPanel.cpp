#include "prpch.h"
#include "ContentBrowserPanel.h"
#include "AssetEditorPanel.h"
#include "Prism/Core/Application.h"
#include "Prism/Core/Input.h"
#include "Prism/Utilities/FileSystem.h"

#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)
PR_WARNING_DISABLE(4267)
PR_WARNING_DISABLE(4244)

namespace Prism {

    ContentBrowserPanel::ContentBrowserPanel()
    {
        AssetManager::SetAssetChangeCallback([&]()
        {
            UpdateCurrentDirectory(m_CurrentDirHandle);
        });

        m_FileTex = Texture2D::Create("assets/editor/file.png");
        m_AssetIconMap[""] = Texture2D::Create("assets/editor/folder.png");
        m_AssetIconMap["fbx"] = Texture2D::Create("assets/editor/fbx.png");
        m_AssetIconMap["obj"] = Texture2D::Create("assets/editor/obj.png");
        m_AssetIconMap["wav"] = Texture2D::Create("assets/editor/wav.png");
        m_AssetIconMap["cs"] = Texture2D::Create("assets/editor/csc.png");
        m_AssetIconMap["py"] = Texture2D::Create("assets/editor/pyc.png");
        m_AssetIconMap["png"] = Texture2D::Create("assets/editor/png.png");
        m_AssetIconMap["blend"] = Texture2D::Create("assets/editor/blend.png");
        m_AssetIconMap["psc"] = Texture2D::Create("assets/editor/prism.png");
        m_AssetIconMap["Shader"] = Texture2D::Create("assets/editor/shader.png");

        m_BackbtnTex = Texture2D::Create("assets/editor/btn_back.png");
        m_FwrdbtnTex = Texture2D::Create("assets/editor/btn_fwrd.png");
        m_FolderRightTex = Texture2D::Create("assets/editor/folder_hierarchy.png");
        m_SearchTex = Texture2D::Create("assets/editor/search.png");

        m_BaseDirectoryHandle = AssetManager::GetAssetHandleFromFilePath("Assets");
        m_BaseDirectory = AssetManager::GetAsset<Directory>(m_BaseDirectoryHandle);
        UpdateCurrentDirectory(m_BaseDirectoryHandle);

        memset(m_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
        memset(m_SearchBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
    }

    void ContentBrowserPanel::DrawDirectoryInfo(AssetHandle directory)
    {
        const Ref<Directory>& dir = AssetManager::GetAsset<Directory>(directory);

        if (ImGui::TreeNode(dir->FileName.c_str()))
        {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                UpdateCurrentDirectory(directory);

            for (AssetHandle child : dir->ChildDirectories)
            {
                DrawDirectoryInfo(child);
            }

            ImGui::TreePop();
        }
    }

    static int s_ColumnCount = 10;
    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Project", NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
        {
            UI::BeginPropertyGrid();
            ImGui::SetColumnOffset(1, 240);

            ImGui::BeginChild("##folders_common");
            {
                if (ImGui::CollapsingHeader("Content", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (AssetHandle child : m_BaseDirectory->ChildDirectories)
                    {
                        DrawDirectoryInfo(child);
                    }
                }
            }
            ImGui::EndChild();

            ImGui::NextColumn();

            ImGui::BeginChild("##directory_structure", ImVec2(ImGui::GetColumnWidth() - 12, ImGui::GetWindowHeight() - 60));
            {
                ImGui::BeginChild("##directory_breadcrumbs", ImVec2(ImGui::GetColumnWidth() - 100, 30));
                RenderBreadCrumbs();
                ImGui::EndChild();

                ImGui::BeginChild("Scrolling");

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2F, 0.205F, 0.21F, 0.25F));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15F, 0.15F, 0.15F, 0.25F));

                if (Input::IsKeyPressed(KeyCode::Escape) || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_IsAnyItemHovered))
                {
                    m_SelectedAssets.Clear();
                    m_RenamingSelected = false;
                    memset(m_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
                    memset(m_SearchBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
                }

                m_IsAnyItemHovered = false;

                if (ImGui::BeginPopupContextWindow(0, 1))
                {
                    if (ImGui::BeginMenu("Create"))
                    {
                        if (ImGui::MenuItem("Folder"))
                        {
                            bool created = FileSystem::CreateFolder(m_CurrentDirectory->FilePath + "/New Folder");

                            if (created)
                            {
                                UpdateCurrentDirectory(m_CurrentDirHandle);
                                auto createdDirectory = AssetManager::GetAsset<Directory>(AssetManager::GetAssetHandleFromFilePath(m_CurrentDirectory->FilePath + "/New Folder"));
                                m_SelectedAssets.Select(createdDirectory->Handle);
                                memset(m_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
                                memcpy(m_RenameBuffer, createdDirectory->FileName.c_str(), createdDirectory->FileName.size());
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
                            AssetManager::CreateNewAsset<PhysicsMaterial>("New Physics Material.ppm", AssetType::PhysicsMat, m_CurrentDirHandle, 0.6F, 0.6F, 0.0F);
                            UpdateCurrentDirectory(m_CurrentDirHandle);
                        }

                        ImGui::EndMenu();
                    }

                    if (ImGui::MenuItem("Import"))
                    {
                    }

                    if (ImGui::MenuItem("Refresh"))
                    {
                        UpdateCurrentDirectory(m_CurrentDirHandle);
                    }

                    ImGui::EndPopup();
                }

                ImGui::Columns(s_ColumnCount, nullptr, false);

                for (Ref<Asset>& asset : m_CurrentDirFolders)
                {
                    RenderAsset(asset);

                    ImGui::NextColumn();
                }

                for (Ref<Asset>& asset : m_CurrentDirFiles)
                {
                    RenderAsset(asset);

                    ImGui::NextColumn();
                }

                if (m_UpdateDirectoryNextFrame)
                {
                    UpdateCurrentDirectory(m_CurrentDirHandle);
                    m_UpdateDirectoryNextFrame = false;
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

            ImGui::BeginChild("##panel_controls", ImVec2(ImGui::GetColumnWidth() - 12, 20), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::Columns(4, 0, false);
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
            ImGui::SliderInt("##column_count", &s_ColumnCount, 2, 15);
            ImGui::EndChild();

            UI::EndPropertyGrid();
        }
        ImGui::End();
    }

    void ContentBrowserPanel::RenderAsset(Ref<Asset>& asset)
    {
        AssetHandle assetHandle = asset->Handle;
        std::string filename = asset->FileName;

        ImGui::PushID(&asset->Handle);
        ImGui::BeginGroup();

        Ref<Image2D> iconImage = m_AssetIconMap.find(asset->Extension) != m_AssetIconMap.end() ? m_AssetIconMap[asset->Extension]->GetImage() : m_FileTex->GetImage();

        if (m_SelectedAssets.IsSelected(assetHandle))
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25F, 0.25F, 0.25F, 0.75F));

        float buttonWidth = ImGui::GetColumnWidth() - 15.0F;
        UI::ImageButton(iconImage, { buttonWidth, buttonWidth });

        if (m_SelectedAssets.IsSelected(assetHandle))
            ImGui::PopStyleColor();

        HandleDragDrop(iconImage, asset);

        if (ImGui::IsItemHovered())
        {
            m_IsAnyItemHovered = true;

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (asset->Type == AssetType::Directory)
                {
                    m_PrevDirHandle = m_CurrentDirHandle;
                    m_CurrentDirHandle = assetHandle;
                    m_UpdateDirectoryNextFrame = true;
                }
                else
                {
                    AssetEditorPanel::OpenEditor(asset);
                }
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_IsDragging)
            {
                if (!Input::IsKeyPressed(KeyCode::LeftControl))
                    m_SelectedAssets.Clear();

                if (m_SelectedAssets.IsSelected(assetHandle))
                    m_SelectedAssets.Deselect(assetHandle);
                else
                    m_SelectedAssets.Select(assetHandle);
            }
        }

        bool shouldOpenDeleteModal = false;

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Rename"))
            {
                m_SelectedAssets.Select(assetHandle);
                memset(m_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
                memcpy(m_RenameBuffer, filename.c_str(), filename.size());
                m_RenamingSelected = true;
            }

            if (ImGui::MenuItem("Delete"))
                shouldOpenDeleteModal = true;

            ImGui::EndPopup();
        }

        if (shouldOpenDeleteModal)
        {
            ImGui::OpenPopup("Delete Asset");
            shouldOpenDeleteModal = false;
        }

        bool deleted = false;
        if (ImGui::BeginPopupModal("Delete Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (asset->Type == AssetType::Directory)
                ImGui::Text("Are you sure you want to delete %s and everything within it?", filename.c_str());
            else
                ImGui::Text("Are you sure you want to delete %s?", filename.c_str());

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
                deleted = FileSystem::DeleteFile(filepath);
                if (deleted)
                {
                    FileSystem::DeleteFile(filepath + ".meta");
                    AssetManager::RemoveAsset(assetHandle);
                    m_UpdateDirectoryNextFrame = true;
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
            ImGui::SetNextItemWidth(buttonWidth);

            if (!m_SelectedAssets.IsSelected(assetHandle) || !m_RenamingSelected)
                ImGui::TextWrapped(filename.c_str());

            if (m_SelectedAssets.IsSelected(assetHandle))
                HandleRenaming(asset);
        }

        ImGui::EndGroup();
        ImGui::PopID();
    }

    void ContentBrowserPanel::HandleDragDrop(Ref<Image2D> icon, Ref<Asset>& asset)
    {
        if (asset->Type == AssetType::Directory && m_IsDragging)
        {
            if (ImGui::BeginDragDropTarget())
            {
                auto payload = ImGui::AcceptDragDropPayload("asset_payload");
                if (payload)
                {
                    int count = payload->DataSize / sizeof(AssetHandle);

                    for (int i = 0; i < count; i++)
                    {
                        AssetHandle handle = *(((AssetHandle*)payload->Data) + i);
                        Ref<Asset> droppedAsset = AssetManager::GetAsset<Asset>(handle, false);

                        bool result = FileSystem::MoveFile(droppedAsset->FilePath, asset->FilePath);
                        if (result)
                            droppedAsset->ParentDirectory = asset->Handle;
                    }

                    m_UpdateDirectoryNextFrame = true;
                }
            }
        }

        if (!m_SelectedAssets.IsSelected(asset->Handle) || m_IsDragging)
            return;

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped) && ImGui::IsItemClicked(ImGuiMouseButton_Left))
            m_IsDragging = true;

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            UI::Image(icon, ImVec2(20, 20));
            ImGui::SameLine();
            ImGui::Text(asset->FileName.c_str());
            ImGui::SetDragDropPayload("asset_payload", m_SelectedAssets.GetSelectionData(), m_SelectedAssets.SelectionCount() * sizeof(AssetHandle));
            m_IsDragging = true;
            ImGui::EndDragDropSource();
        }
    }

    void ContentBrowserPanel::RenderBreadCrumbs()
    {
        if (UI::ImageButton(m_BackbtnTex->GetImage(), ImVec2(20, 18)))
        {
            if (m_CurrentDirHandle == m_BaseDirectoryHandle) return;
            m_NextDirHandle = m_CurrentDirHandle;
            m_PrevDirHandle = m_CurrentDirectory->ParentDirectory;
            UpdateCurrentDirectory(m_PrevDirHandle);
        }

        ImGui::SameLine();

        if (UI::ImageButton(m_FwrdbtnTex->GetImage(), ImVec2(20, 18)))
        {
            UpdateCurrentDirectory(m_NextDirHandle);
        }

        ImGui::SameLine();

        {
            ImGui::PushItemWidth(200);

            if (ImGui::InputTextWithHint("##Search", "Search...", m_SearchBuffer, MAX_INPUT_BUFFER_LENGTH))
            {
                if (strlen(m_SearchBuffer) == 0)
                {
                    UpdateCurrentDirectory(m_CurrentDirHandle);
                }
                else
                {
                    m_CurrentDirFolders = AssetManager::SearchAssets(m_SearchBuffer, m_CurrentDirectory->FilePath, AssetType::Directory);
                    m_CurrentDirFiles = AssetManager::SearchAssets(m_SearchBuffer, m_CurrentDirectory->FilePath);
                }
            }

            ImGui::PopItemWidth();
        }

        ImGui::SameLine();

        if (m_UpdateBreadCrumbs)
        {
            m_BreadCrumbData.clear();

            AssetHandle currentHandle = m_CurrentDirHandle;
            while (currentHandle != 0)
            {
                Ref<Directory> dirInfo = AssetManager::GetAsset<Directory>(currentHandle);
                m_BreadCrumbData.push_back(dirInfo);
                currentHandle = dirInfo->ParentDirectory;
            }

            std::reverse(m_BreadCrumbData.begin(), m_BreadCrumbData.end());

            m_UpdateBreadCrumbs = false;
        }

        for (int i = 0; i < m_BreadCrumbData.size(); i++)
        {
            if (m_BreadCrumbData[i]->FileName != "Assets")
                ImGui::Text("/");

            ImGui::SameLine();

            int size = strlen(m_BreadCrumbData[i]->FileName.c_str()) * 7;

            if (ImGui::Selectable(m_BreadCrumbData[i]->FileName.c_str(), false, 0, ImVec2(size, 22)))
            {
                UpdateCurrentDirectory(m_BreadCrumbData[i]->Handle);
            }

            ImGui::SameLine();
        }

        ImGui::SameLine();

        ImGui::Dummy(ImVec2(ImGui::GetColumnWidth() - 400, 0));
        ImGui::SameLine();
    }

    void ContentBrowserPanel::HandleRenaming(Ref<Asset>& asset)
    {
        if (m_SelectedAssets.SelectionCount() > 1)
            return;

        if (!m_RenamingSelected && Input::IsKeyPressed(KeyCode::F2))
        {
            memset(m_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
            memcpy(m_RenameBuffer, asset->FileName.c_str(), asset->FileName.size());
            m_RenamingSelected = true;
        }

        if (m_RenamingSelected)
        {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##rename_dummy", m_RenameBuffer, MAX_INPUT_BUFFER_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                PR_CORE_INFO("Renaming to {0}", m_RenameBuffer);
                AssetManager::Rename(asset->Handle, m_RenameBuffer);
                m_RenamingSelected = false;
                m_SelectedAssets.Clear();
                m_UpdateDirectoryNextFrame = true;
            }
        }
    }

    void ContentBrowserPanel::UpdateCurrentDirectory(AssetHandle directoryHandle)
    {
        m_UpdateBreadCrumbs = true;
        m_CurrentDirFiles.clear();
        m_CurrentDirFolders.clear();

        m_CurrentDirHandle = directoryHandle;
        m_CurrentDirectory = AssetManager::GetAsset<Directory>(m_CurrentDirHandle);

        std::vector<Ref<Asset>> assets = AssetManager::GetAssetsInDirectory(m_CurrentDirHandle);
        for (auto& asset : assets)
        {
            if (asset->Type == AssetType::Directory)
                m_CurrentDirFolders.push_back(asset);
            else
                m_CurrentDirFiles.push_back(asset);
        }
    }

}
