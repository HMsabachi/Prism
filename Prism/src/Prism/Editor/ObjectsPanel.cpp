#include "prpch.h"
#include "ObjectsPanel.h"
#include "Prism/ImGui/ImGui.h"

#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)

namespace Prism {

    ObjectsPanel::ObjectsPanel()
    {
        m_CubeImage = Texture2D::Create("Assets/editor/asset.png");
    }

    void ObjectsPanel::DrawObject(const char* label, AssetHandle handle)
    {
        ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(30, 30));
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
        ImGui::Selectable(label);

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(20, 20));
            ImGui::SameLine();

            ImGui::Text(label);

            ImGui::SetDragDropPayload("asset_payload", &handle, sizeof(AssetHandle));
            ImGui::EndDragDropSource();
        }
    }

    void ObjectsPanel::OnImGuiRender()
    {
        static const AssetHandle CubeHandle = AssetManager::GetAssetIDForFile("Assets/meshes/Default/Cube.fbx");
        static const AssetHandle SphereHandle = AssetManager::GetAssetIDForFile("Assets/meshes/Default/Sphere.fbx");
        static const AssetHandle CylinderHandle = AssetManager::GetAssetIDForFile("Assets/meshes/Default/Cylinder.fbx");
        static const AssetHandle TorusHandle = AssetManager::GetAssetIDForFile("Assets/meshes/Default/Torus.fbx");
        static const AssetHandle PlaneHandle = AssetManager::GetAssetIDForFile("Assets/meshes/Default/Plane.fbx");
        static const AssetHandle ConeHandle = AssetManager::GetAssetIDForFile("Assets/meshes/Default/Cone.fbx");

        ImGui::Begin("Objects");
        {
            ImGui::BeginChild("##objects_window");
            DrawObject("Cube", CubeHandle);
            DrawObject("Sphere", SphereHandle);
            DrawObject("Cylinder", CylinderHandle);
            DrawObject("Torus", TorusHandle);
            DrawObject("Plane", PlaneHandle);
            DrawObject("Cone", ConeHandle);
            ImGui::EndChild();
        }

        ImGui::End();
    }

}
