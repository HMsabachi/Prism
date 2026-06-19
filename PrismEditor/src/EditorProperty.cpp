#include "EditorProperty.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)


namespace Prism
{
    bool Property(const std::string& name, glm::vec4& value, float min, float max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text(name.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + name;
        if ((int)flags & (int)PropertyFlag::ColorProperty)
            modified = ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
        else if ((int)flags & (int)PropertyFlag::SliderProperty)
            modified = ImGui::SliderFloat4(id.c_str(), glm::value_ptr(value), min, max);
        else
            modified = ImGui::DragFloat4(id.c_str(), glm::value_ptr(value));

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Property(const std::string& name, glm::vec2& value, PropertyFlag flags)
    {
        return Property(name, value, -1.0f, 1.0f, flags);
    }

    bool Property(const std::string& name, glm::vec2& value, float min, float max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text(name.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + name;
        if ((int)flags & (int)PropertyFlag::SliderProperty)
            modified = ImGui::SliderFloat2(id.c_str(), glm::value_ptr(value), min, max);
        else
            modified = ImGui::DragFloat2(id.c_str(), glm::value_ptr(value));

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Property(const std::string& name, glm::vec4& value, PropertyFlag flags)
    {
        return Property(name, value, -1.0f, 1.0f, flags);
    }

    bool Property(const std::string& name, glm::vec3& value, float min, float max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text(name.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + name;
        if ((int)flags & (int)PropertyFlag::ColorProperty)
            modified = ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
        else if ((int)flags & (int)PropertyFlag::SliderProperty)
            modified = ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max);
        else
            modified = ImGui::DragFloat3(id.c_str(), glm::value_ptr(value));

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Property(const std::string& name, glm::vec3& value, PropertyFlag flags)
    {
        return Property(name, value, -1.0f, 1.0f, flags);
    }

    bool Property(const std::string& name, float& value, float min, float max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text(name.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + name;
        if ((int)flags & (int)PropertyFlag::SliderProperty)
            modified = ImGui::SliderFloat(id.c_str(), &value, min, max);
        else
            modified = ImGui::DragFloat(id.c_str(), &value);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Property(const std::string& name, int& value, int min, int max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text(name.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + name;
        if ((int)flags & (int)PropertyFlag::SliderProperty)
            modified = ImGui::SliderInt(id.c_str(), &value, min, max);
        else
            modified = ImGui::DragInt(id.c_str(), &value);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Property(const std::string& name, const Ref<Texture2D>& texture, uint32_t fallbackRendererID)
     {
        ImGui::Text("%s", name.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
        uint32_t texID = texture ? texture->GetRendererID() : fallbackRendererID;
        ImGui::Image((void*)texID, ImVec2(64, 64));
        ImGui::PopStyleVar();

        bool clicked = false;
        if (ImGui::IsItemHovered())
        {
            if (texture)
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(texture->GetPath().c_str());
                ImGui::PopTextWrapPos();
                ImGui::Image((void*)texture->GetRendererID(), ImVec2(384, 384));
                ImGui::EndTooltip();
            }
            if (ImGui::IsItemClicked())
                clicked = true;
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return clicked;
    }

    bool Property(const std::string& name, const Ref<TextureCube>& texture, uint32_t fallbackRendererID)
    {
        ImGui::Text("%s", name.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
        ImGui::Image((void*)(intptr_t)fallbackRendererID, ImVec2(64, 64));
        ImGui::PopStyleVar();

        if (ImGui::IsItemHovered() && texture)
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(texture->GetPath().c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return false;
    }

    bool Property(const std::string& name, bool& value)
    {
        ImGui::Text(name.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + name;
        bool result = ImGui::Checkbox(id.c_str(), &value);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return result;
    }
}
