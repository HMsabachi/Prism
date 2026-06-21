#include "prpch.h"
#include "PhysicsSettingsWindow.h"
#include "Prism/Physics/Physics.h"
#include "Prism/Physics/PhysicsLayer.h"
#include "Prism/Core/Time.h"
#include "Prism/Core/LanguageManager.h"
#include "Prism/ImGui/ImGui.h"

namespace Prism {

    static int32_t s_SelectedLayer = -1;
    static char s_NewLayerNameBuffer[50];

    void PhysicsSettingsWindow::OnImGuiRender(bool& show)
    {
        if (!show)
            return;

        ImGui::Begin("Physics", &show);
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
        PhysicsSettings& settings = Physics::GetSettings();

        UI::Property(TR("Fixed Timestep (Default 0.0167)"), settings.FixedTimestep, 0.001F, 0.1F);
        UI::Property(TR("Gravity (Default -9.81)"), settings.Gravity.y, -50.0F, 50.0F);

        static const char* broadphaseTypeStrings[] = { TR("Sweep And Prune"), TR("Multi Box Pruning"), TR("Automatic Box Pruning") };
        UI::Property(TR("Broadphase Type"), broadphaseTypeStrings, 3, (int*)&settings.BroadphaseAlgorithm);

        if (settings.BroadphaseAlgorithm != BroadphaseType::AutomaticBoxPrune)
        {
            UI::Property(TR("World Bounds (Min)"), settings.WorldBoundsMin);
            UI::Property(TR("World Bounds (Max)"), settings.WorldBoundsMax);
            UI::Property(TR("Grid Subdivisions"), settings.WorldBoundsSubdivisions, 1u, 10000u);
        }
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

        uint32_t buttonId = 0;

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
