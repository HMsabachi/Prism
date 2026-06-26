#pragma once
#include "Prism/Core/Core.h"
#include <glm/glm.hpp>

namespace Prism::Math {

    bool PRISM_API DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

}
