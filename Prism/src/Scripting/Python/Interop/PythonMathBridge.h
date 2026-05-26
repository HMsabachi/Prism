#pragma once
#include <string>
#include <string_view>
#include <cstdint>
#include <glm/glm.hpp>

namespace Prism::Python {

    class ScriptRef;
    struct ScriptValue;

    // ── 初始化（缓存 PyGLM 类型）──
    void InitializeMathBridge();

    // ════════════════════════════════════════════
    //  C++ → Python（返回 ScriptRef）
    // ════════════════════════════════════════════

    ScriptRef FloatToValue(float v);
    ScriptRef IntToValue(int32_t v);
    ScriptRef UInt64ToValue(uint64_t v);
    ScriptRef StringToValue(const std::string_view v);
    ScriptRef BoolToValue(bool v);
    ScriptRef NoneValue();

    ScriptRef Vec2ToValue(const glm::vec2& v);
    ScriptRef Vec3ToValue(const glm::vec3& v);
    ScriptRef Vec4ToValue(const glm::vec4& v);
    ScriptRef Mat4ToValue(const glm::mat4& m);

    // ════════════════════════════════════════════
    //  Python → C++
    // ════════════════════════════════════════════

    float       ValueToFloat(const ScriptRef& v);
    int32_t     ValueToInt(const ScriptRef& v);
    uint64_t    ValueToUInt64(const ScriptRef& v);
    std::string ValueToString(const ScriptRef& v);
    bool        ValueToBool(const ScriptRef& v);

    glm::vec2   ValueToVec2(const ScriptRef& obj);
    glm::vec3   ValueToVec3(const ScriptRef& obj);
    glm::vec4   ValueToVec4(const ScriptRef& obj);
    glm::mat4   ValueToMat4(const ScriptRef& obj);

    // ════════════════════════════════════════════
    //  Tuple 操作
    // ════════════════════════════════════════════

    ScriptRef   MakeTuple(const ScriptRef* elements, uint32_t count);
    uint32_t    GetTupleSize(const ScriptRef& tuple);
    ScriptRef   GetTupleElement(const ScriptRef& tuple, uint32_t index);

} // namespace Prism::Python
