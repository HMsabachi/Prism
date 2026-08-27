#include "prpch.h"
#include "ImGui.h"
#include "Prism/Renderer/Texture.h"
#include "Platform/OpenGL/OpenGLImage.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "backends/imgui_impl_vulkan.h"

#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)

namespace Prism {
namespace UI {

    static int s_UIContextID = 0;
    static uint32_t s_Counter = 0;
    static char s_IDBuffer[16];
    static int s_CheckboxCount = 0;

    inline static ImTextureID GetImGuiTextureID(const Ref<Image2D>& image)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:
                PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPIType::OpenGL:
                return (ImTextureID)(uint64_t)image.As<OpenGLImage2D>()->GetRendererID();
            case RendererAPIType::Vulkan:
            {
                Ref<VulkanImage2D> vulkanImage = image.As<VulkanImage2D>();
                const auto& imageInfo = vulkanImage->GetImageInfo();
                if (!imageInfo.ImageView)
                    return nullptr;
                auto textureID = ImGui_ImplVulkan_AddTexture(imageInfo.Sampler, imageInfo.ImageView, vulkanImage->GetDescriptor().imageLayout);
                return (ImTextureID)(uint64_t)textureID;
            }
        }
        return nullptr;
    }

    void PushID()
    {
        ImGui::PushID(s_UIContextID++);
        s_Counter = 0;
    }

    void PopID()
    {
        ImGui::PopID();
        s_UIContextID--;
    }

    void BeginPropertyGrid()
    {
        PushID();
        ImGui::Columns(2);
    }

    void EndPropertyGrid()
    {
        ImGui::Columns(1);
        PopID();
    }

    bool BeginTreeNode(const char* label, bool defaultOpen)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
        if (defaultOpen)
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        return ImGui::TreeNodeEx(label, flags);
    }

    void EndTreeNode()
    {
        ImGui::TreePop();
    }

    // ── Standard types ──

    bool Property(const std::string& label, bool& value)
    {
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        bool result = ImGui::Checkbox(("##" + label).c_str(), &value);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return result;
    }

    bool Property(const std::string& label, int& value, int min, int max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + label;
        if ((int)flags & (int)PropertyFlag::SliderProperty)
            modified = ImGui::SliderInt(id.c_str(), &value, min, max);
        else
            modified = ImGui::DragInt(id.c_str(), &value);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Property(const std::string& label, uint32_t& value, uint32_t min, uint32_t max, PropertyFlag flags)
    {
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + label;
        bool changed = ImGui::DragInt(id.c_str(), (int*)&value, 1.0F, (int)min, (int)max);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return changed;
    }

    bool Property(const std::string& label, float& value, float min, float max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + label;
        if ((int)flags & (int)PropertyFlag::SliderProperty)
            modified = ImGui::SliderFloat(id.c_str(), &value, min, max);
        else
            modified = ImGui::DragFloat(id.c_str(), &value);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Property(const std::string& label, float& value, float delta, float min, float max, PropertyFlag flags, bool readOnly)
    {
        bool modified = false;
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + label;

        if (readOnly)
        {
            ImGui::InputFloat(id.c_str(), &value, 0.0F, 0.0F, "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        else if ((int)flags & (int)PropertyFlag::SliderProperty)
        {
            modified = ImGui::SliderFloat(id.c_str(), &value, min, max);
        }
        else
        {
            modified = ImGui::DragFloat(id.c_str(), &value, delta, min, max);
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    // ── Vector types ──

    bool Property(const std::string& label, glm::vec2& value, PropertyFlag flags)
    {
        return Property(label, value, -1.0f, 1.0f, flags);
    }

    bool Property(const std::string& label, glm::vec2& value, float min, float max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + label;
        if ((int)flags & (int)PropertyFlag::SliderProperty)
            modified = ImGui::SliderFloat2(id.c_str(), glm::value_ptr(value), min, max);
        else
            modified = ImGui::DragFloat2(id.c_str(), glm::value_ptr(value));

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Property(const std::string& label, glm::vec3& value, PropertyFlag flags)
    {
        return Property(label, value, -1.0f, 1.0f, flags);
    }

    bool Property(const std::string& label, glm::vec3& value, float min, float max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + label;
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

    bool Property(const std::string& label, glm::vec4& value, PropertyFlag flags)
    {
        return Property(label, value, -1.0f, 1.0f, flags);
    }

    bool Property(const std::string& label, glm::vec4& value, float min, float max, PropertyFlag flags)
    {
        bool modified = false;
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        std::string id = "##" + label;
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

    // ── String types ──

    bool Property(const std::string& label, std::string& value, bool error)
    {
        bool modified = false;

        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        char buffer[256];
        strcpy(buffer, value.c_str());

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        _itoa(s_Counter++, s_IDBuffer + 2, 16);

        if (error)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        if (ImGui::InputText(s_IDBuffer, buffer, 256))
        {
            value = buffer;
            modified = true;
        }
        if (error)
            ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        return modified;
    }

    bool Property(const std::string& label, const char* value)
    {
        bool modified = false;
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        _itoa(s_Counter++, s_IDBuffer + 2, 16);
        modified = ImGui::InputText(s_IDBuffer, (char*)value, 256, ImGuiInputTextFlags_ReadOnly);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    // ── Texture types ──

    void Image(const Ref<::Prism::Image2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& b_color)
    {
        auto texID = GetImGuiTextureID(image);
        ImGui::Image((void*)(intptr_t)texID, size, uv0, uv1);
    }

    bool ImageButton(const Ref<::Prism::Image2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
    {
        auto texID = GetImGuiTextureID(image);
        return ImGui::ImageButton((void*)(intptr_t)texID, size, uv0, uv1, frame_padding, bg_col, tint_col);
    }

    PRISM_API bool Property(const std::string& label, const Ref<Texture2D>& texture, const Ref<::Prism::Image2D> fallback)
    {
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
        auto texID = texture ? GetImGuiTextureID(texture->GetImage()) : GetImGuiTextureID(fallback);
        ImGui::Image(texID, ImVec2(64, 64));
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
                ImGui::Image(texID, ImVec2(384, 384));
                ImGui::EndTooltip();
            }
            if (ImGui::IsItemClicked())
                clicked = true;
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return clicked;
    }
  

    // ── Combo ──

    bool Property(const std::string& label, const char** options, int32_t optionCount, int32_t* selected)
    {
        const char* current = options[*selected];
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        bool changed = false;
        std::string id = "##" + label;
        if (ImGui::BeginCombo(id.c_str(), current))
        {
            for (int i = 0; i < optionCount; i++)
            {
                bool is_selected = (current == options[i]);
                if (ImGui::Selectable(options[i], is_selected))
                {
                    current = options[i];
                    *selected = i;
                    changed = true;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return changed;
    }


    PRISM_API bool PropertySlider(const std::string& label, int& value, int min, int max)
    {
        bool modified = false;

        ImGui::Text(label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        _itoa(s_Counter++, s_IDBuffer + 2, 16);
        modified = ImGui::SliderInt(s_IDBuffer, &value, min, max);
        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;

    }

    PRISM_API bool PropertySlider(const std::string& label, float& value, float min, float max)
    {
        bool modified = false;

        ImGui::Text(label.c_str());
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        _itoa(s_Counter++, s_IDBuffer + 2, 16);
        modified = ImGui::SliderFloat(s_IDBuffer, &value, min, max);
        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    // ── Color ──

    bool PropertyColor(const std::string& label, glm::vec3& values)
    {
        return Property(label, values, 0.0f, 1.0f, PropertyFlag::ColorProperty);
    }

    bool PropertyColor(const std::string& label, glm::vec4& values)
    {
        return Property(label, values, 0.0f, 1.0f, PropertyFlag::ColorProperty);
    }

    // ── DrawVec3Control ──

    bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth)
    {
        bool modified = false;

        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
        {
            values.x = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        modified |= ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            values.y = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        modified |= ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
        {
            values.z = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        modified |= ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();

        return modified;
    }

    void BeginCheckboxGroup(const char* label)
    {
        ImGui::Text("%s", label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
    }

    bool PropertyCheckboxGroup(const char* label, bool& value)
    {
        bool modified = false;

        if (++s_CheckboxCount > 1)
            ImGui::SameLine();

        ImGui::Text("%s", label);
        ImGui::SameLine();

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        _itoa(s_Counter++, s_IDBuffer + 2, 16);
        if (ImGui::Checkbox(s_IDBuffer, &value))
            modified = true;

        return modified;
    }

    void EndCheckboxGroup()
    {
        ImGui::PopItemWidth();
        ImGui::NextColumn();
        s_CheckboxCount = 0;
    }

} // namespace UI
} // namespace Prism
