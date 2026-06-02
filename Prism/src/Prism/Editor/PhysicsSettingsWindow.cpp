#include "prpch.h"
#include "PhysicsSettingsWindow.h"
#include "Prism/Physics/Physics.h"

#include "imgui.h"

namespace Prism {

	static int32_t s_SelectedLayer = -1;
	static char s_NewLayerNameBuffer[50];

	void PhysicsSettingsWindow::OnImGuiRender(bool* show)
	{
		if (!(*show))
			return;

		ImGui::Begin("Physics", show);
		ImGui::PushID(0);
		ImGui::Columns(2);

		RenderLayerList();
		ImGui::NextColumn();
		RenderSelectedLayer();

		ImGui::PopID();
		ImGui::End();
	}

	void PhysicsSettingsWindow::RenderLayerList()
	{
		if (ImGui::Button("New Layer"))
		{
			ImGui::OpenPopup("NewLayerNamePopup");
		}

		if (ImGui::BeginPopup("NewLayerNamePopup"))
		{
			ImGui::InputText("##LayerNameID", s_NewLayerNameBuffer, 50);

			if (ImGui::Button("Add"))
			{
				PhysicsLayerManager::AddLayer(std::string(s_NewLayerNameBuffer));
				memset(s_NewLayerNameBuffer, 0, 50);
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		for (const auto& layer : PhysicsLayerManager::GetLayers())
		{
			if (ImGui::Button(layer.Name.c_str()))
			{
				s_SelectedLayer = layer.LayerID;
			}

			if (layer.Name != "Default")
			{
				ImGui::SameLine();
				if (ImGui::Button("X"))
				{
					PhysicsLayerManager::RemoveLayer(layer.LayerID);
				}
			}
		}
	}

	static std::string s_IDString = "##";

	void PhysicsSettingsWindow::RenderSelectedLayer()
	{
		if (s_SelectedLayer == -1)
			return;

		const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(s_SelectedLayer);

		for (const auto& layer : PhysicsLayerManager::GetLayers())
		{
			if (layer.LayerID == s_SelectedLayer)
				continue;

			const PhysicsLayer& otherLayerInfo = PhysicsLayerManager::GetLayer(layer.LayerID);
			bool shouldCollide;

			if (layerInfo.CollidesWith == 0 || otherLayerInfo.CollidesWith == 0)
			{
				shouldCollide = false;
			}
			else
			{
				shouldCollide = layerInfo.CollidesWith & otherLayerInfo.BitValue;
			}

			ImGui::TextUnformatted(otherLayerInfo.Name.c_str());
			ImGui::SameLine();
			if (ImGui::Checkbox((s_IDString + otherLayerInfo.Name).c_str(), &shouldCollide))
			{
				PhysicsLayerManager::SetLayerCollision(s_SelectedLayer, layer.LayerID, shouldCollide);
			}
		}
	}

}
