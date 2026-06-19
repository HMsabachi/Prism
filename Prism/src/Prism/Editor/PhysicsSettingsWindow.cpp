#include "prpch.h"
#include "PhysicsSettingsWindow.h"
#include "Prism/Physics/Physics.h"
#include "Prism/Physics/PhysicsLayer.h"
#include "Prism/Core/Time.h"

#include "imgui.h"
#include "imgui_internal.h"

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
		RenderWorldSettings();
		ImGui::EndColumns();
		ImGui::PopID();

		ImGui::Separator();

		ImGui::PushID(1);
		ImGui::Columns(2);
		RenderLayerList();
		ImGui::NextColumn();
		RenderSelectedLayer();
		ImGui::EndColumns();
		ImGui::PopID();

		ImGui::End();
	}

	void PhysicsSettingsWindow::RenderWorldSettings()
	{
		float timestep = Time::GetFixedDeltaTime();
		if (Property("Fixed Timestep (Default: 0.0167)", timestep, 0.001F, 0.1F))
		{
			Time::SetFixedDeltaTime(timestep);
		}

		float gravity = Physics::GetGravity();
		if (Property("Gravity (Default: -9.81)", gravity, -50.0F, 50.0F))
		{
			Physics::SetGravity(gravity);
		}
	}

	bool PhysicsSettingsWindow::Property(const char* label, float& value, float min, float max)
	{
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + std::string(label);
		bool changed = ImGui::SliderFloat(id.c_str(), &value, min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return changed;
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

		uint32_t buttonId = 1;

		for (const auto& layer : PhysicsLayerManager::GetLayers())
		{
			if (ImGui::Button(layer.Name.c_str()))
			{
				s_SelectedLayer = layer.LayerID;
			}

			if (layer.Name != "Default")
			{
				ImGui::SameLine();
				ImGui::PushID(buttonId++);
				if (ImGui::Button("X"))
				{
					PhysicsLayerManager::RemoveLayer(layer.LayerID);
				}
				ImGui::PopID();
			}
		}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
	{
		s_SelectedLayer = -1;
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
