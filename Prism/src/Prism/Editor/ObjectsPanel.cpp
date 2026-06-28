#include "prpch.h"
#include "ObjectsPanel.h"
#include "Prism/Utilities/DragDropData.h"
#include "Prism/ImGui/ImGui.h"

#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)

namespace Prism {

	ObjectsPanel::ObjectsPanel()
	{
		m_CubeImage = Texture2D::Create("assets/editor/asset.png");
	}

	void ObjectsPanel::OnImGuiRender()
	{
		ImGui::Begin("Objects", NULL, ImGuiWindowFlags_None);
		{
			char buff[100] = { 0 };
			const char* inputHint = "Start Typing To Search";
			ImGui::PushItemWidth(ImGui::GetWindowWidth() - 20);
			ImGui::InputTextWithHint("##SearchObjects", inputHint, buff, 100);

			ImGui::BeginChild("##objects_window");
			{
				ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(30, 30));
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
				ImGui::Selectable("Cube");

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(20, 20));
					ImGui::SameLine();

					ImGui::Text("Cube");

					DragDropData data("Mesh", "assets/meshes/Default/Cube.fbx", "Cube");
					ImGui::SetDragDropPayload("scene_entity_objectP", &data, sizeof(data));
					ImGui::EndDragDropSource();
				}

				ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(30, 30));
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
				ImGui::Selectable("Sphere");

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(20, 20));
					ImGui::SameLine();

					ImGui::Text("Sphere");
					DragDropData data("Mesh", "assets/meshes/Default/Sphere.fbx", "Sphere");
					ImGui::SetDragDropPayload("scene_entity_objectP", &data, sizeof(data));
					ImGui::EndDragDropSource();
				}

				ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(30, 30));
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
				ImGui::Selectable("Cylinder");

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(20, 20));
					ImGui::SameLine();

					ImGui::Text("Cylinder");
					DragDropData data("Mesh", "assets/meshes/Default/Cylinder.fbx", "Cylinder");
					ImGui::SetDragDropPayload("scene_entity_objectP", &data, sizeof(data));
					ImGui::EndDragDropSource();
				}

				ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(30, 30));
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
				ImGui::Selectable("Torus");

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(20, 20));
					ImGui::SameLine();

					ImGui::Text("Torus");
					DragDropData data("Mesh", "assets/meshes/Default/Torus.fbx", "Torus");
					ImGui::SetDragDropPayload("scene_entity_objectP", &data, sizeof(data));
					ImGui::EndDragDropSource();
				}


				ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(30, 30));
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
				ImGui::Selectable("Plane");

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(20, 20));
					ImGui::SameLine();

					ImGui::Text("Plane");
					DragDropData data("Mesh", "assets/meshes/Default/Plane.fbx", "Plane");
					ImGui::SetDragDropPayload("scene_entity_objectP", &data, sizeof(data));
					ImGui::EndDragDropSource();
				}

				ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(30, 30));
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
				ImGui::Selectable("Cone");

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					ImGui::Image((ImTextureID)m_CubeImage->GetRendererID(), ImVec2(20, 20));
					ImGui::SameLine();

					ImGui::Text("Cone");
					DragDropData data("Mesh", "assets/meshes/Default/Cone.fbx", "Cone");
					ImGui::SetDragDropPayload("scene_entity_objectP", &data, sizeof(data));
					ImGui::EndDragDropSource();
				}

				ImGui::EndChild();
			}
		}

		ImGui::End();
	}

}
