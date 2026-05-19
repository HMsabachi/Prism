#pragma once

#include <string>
#include <glm/glm.hpp>

#include "imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Prism/Core/Ref.h"


namespace Prism
{
    class Texture2D;
    class TextureCube;
    enum class PropertyFlag
    {
        None = 0, ColorProperty = 1, DragProperty = 2, SliderProperty = 4
    };

    bool Property(const std::string& name, bool& value);
    bool Property(const std::string& name, int& value, int min = -10, int max = 10, PropertyFlag flags = PropertyFlag::None);
    bool Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
    bool Property(const std::string& name, glm::vec2& value, PropertyFlag flags);
    bool Property(const std::string& name, glm::vec2& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
    bool Property(const std::string& name, glm::vec3& value, PropertyFlag flags);
    bool Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
    bool Property(const std::string& name, glm::vec4& value, PropertyFlag flags);
    bool Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
    bool Property(const std::string& name, const Ref<Texture2D>& texture, uint32_t fallbackRendererID);
    bool Property(const std::string& name, const Ref<TextureCube>& texture, uint32_t fallbackRendererID);
}
