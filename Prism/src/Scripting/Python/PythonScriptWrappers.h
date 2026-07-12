#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Physics/Physics.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <optional>
#include <vector>

namespace pybind11 { class object; }

namespace Prism
{
    class Mesh;
    class Texture2D;
    class Material;
    struct RaycastHit;
    struct OverlapHitData;
    enum class KeyCode : uint16_t;
    enum class MouseButton : uint16_t;
    enum class CursorMode;
}

namespace Prism::PythonScript
{

} // namespace Prism::PythonScript
