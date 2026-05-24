#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Core/UUID.h"
#include "Prism/Core/Buffer.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Prism
{

    enum class ScriptFieldType : uint8_t
    {
        None = 0,
        Float, Double,
        Bool,
        Int8, Int16, Int32, Int64,
        UInt8, UInt16, UInt32, UInt64,
        String,
        Vector2, Vector3, Vector4,
        Object
    };

    struct PRISM_API ScriptFieldMetadata
    {
        std::string Name;
        ScriptFieldType Type = ScriptFieldType::None;
        Buffer DefaultValue;
    };

    struct PRISM_API ScriptClassMetadata
    {
        UUID ScriptID;
        std::string FullName;
        std::string ModuleName;
        std::string ClassName;
        std::unordered_map<uint32_t, ScriptFieldMetadata> Fields;  // key = FNV hash of field name
    };

    // ── Python 标注 → ScriptFieldType 映射 ──
    PRISM_API ScriptFieldType GetFieldTypeFromPythonAnnotation(const std::string& annotation);

}
