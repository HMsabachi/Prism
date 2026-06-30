#include "prpch.h"
#include "ObjectsPanel.h"
#include "Prism/ImGui/ImGui.h"

#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)

namespace Prism {

    ObjectsPanel::ObjectsPanel()
    {
        m_CubeImage = Texture2D::Create("assets/editor/asset.png");
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

            ImGui::SetDragDropPayload("scene_entity_assetsP", &handle, sizeof(AssetHandle));
            ImGui::EndDragDropSource();
        }
    }

    void ObjectsPanel::OnImGuiRender()
    {
        static const AssetHandle CubeHandle = AssetManager::GetAssetIDForFile("assets/meshes/Default/Cube.fbx");
        static const AssetHandle SphereHandle = AssetManager::GetAssetIDForFile("assets/meshes/Default/Sphere.fbx");
        static const AssetHandle CylinderHandle = AssetManager::GetAssetIDForFile("assets/meshes/Default/Cylinder.fbx");
        static const AssetHandle TorusHandle = AssetManager::GetAssetIDForFile("assets/meshes/Default/Torus.fbx");
        static const AssetHandle PlaneHandle = AssetManager::GetAssetIDForFile("assets/meshes/Default/Plane.fbx");
        static const AssetHandle ConeHandle = AssetManager::GetAssetIDForFile("assets/meshes/Default/Cone.fbx");

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
