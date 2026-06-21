#pragma once

#include <string>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include "Prism/Core/Ref.h"
#include "Prism/Core/Core.h"

namespace Prism {
    class Texture2D;
    class TextureCube;

    namespace UI {

        enum class PRISM_API PropertyFlag
        {
            None = 0, ColorProperty = 1, DragProperty = 2, SliderProperty = 4
        };

        PRISM_API void PushID();
        PRISM_API void PopID();

        PRISM_API void BeginPropertyGrid();
        PRISM_API void EndPropertyGrid();

        PRISM_API bool BeginTreeNode(const char* label, bool defaultOpen = true);
        PRISM_API void EndTreeNode();

        PRISM_API bool Property(const std::string& label, bool& value);
        PRISM_API bool Property(const std::string& label, int& value, int min = -10, int max = 10, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, uint32_t& value, uint32_t min = 0, uint32_t max = 10000, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, float& value, float delta, float min, float max, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, glm::vec2& value, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, glm::vec2& value, float min, float max, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, glm::vec3& value, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, glm::vec3& value, float min, float max, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, glm::vec4& value, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, glm::vec4& value, float min, float max, PropertyFlag flags = PropertyFlag::None);
        PRISM_API bool Property(const std::string& label, std::string& value, bool error = false);
        PRISM_API bool Property(const std::string& label, const char* value);
        PRISM_API bool Property(const std::string& label, const Ref<Texture2D>& texture, uint32_t fallbackRendererID);
        PRISM_API bool Property(const std::string& label, const Ref<TextureCube>& texture, uint32_t fallbackRendererID);
        PRISM_API bool Property(const std::string& label, const char** options, int32_t optionCount, int32_t* selected);

        PRISM_API bool PropertyColor(const std::string& label, glm::vec3& values);
        PRISM_API bool PropertyColor(const std::string& label, glm::vec4& values);

        PRISM_API bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

    }
}
