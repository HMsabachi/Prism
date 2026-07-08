#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Core/UUID.h"
#include "Prism/Core/Buffer.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Rolky { class Type; }

namespace Prism
{

    enum class ScriptFieldType : uint16_t
    {
        None = 0,
        Float, Double,
        Bool,
        Int8, Int16, Int32, Int64,
        UInt8, UInt16, UInt32, UInt64,
        Vector2, Vector3, Vector4,
        Object,
        MeshRef, MaterialRef, Texture2DRef
    };

    inline uint64_t DataTypeSize(ScriptFieldType type)
    {
        switch (type)
        {
        case Prism::ScriptFieldType::None: PR_CORE_ASSERT(false, "未注册的类型");
        case Prism::ScriptFieldType::Float: return sizeof(float);
        case Prism::ScriptFieldType::Double: return sizeof(double);
        case Prism::ScriptFieldType::Bool: return sizeof(bool);
        case Prism::ScriptFieldType::Int8: return sizeof(int8_t);
        case Prism::ScriptFieldType::Int16: return sizeof(int16_t);
        case Prism::ScriptFieldType::Int32: return sizeof(int32_t);
        case Prism::ScriptFieldType::Int64: return sizeof(int64_t);
        case Prism::ScriptFieldType::UInt8: return sizeof(uint8_t);
        case Prism::ScriptFieldType::UInt16: return sizeof(uint16_t);
        case Prism::ScriptFieldType::UInt32: return sizeof(uint32_t);
        case Prism::ScriptFieldType::UInt64: return sizeof(uint64_t);
        case Prism::ScriptFieldType::Vector2: return sizeof(glm::vec2);
        case Prism::ScriptFieldType::Vector3: return sizeof(glm::vec3);
        case Prism::ScriptFieldType::Vector4: return sizeof(glm::vec4);
        case Prism::ScriptFieldType::Object: return sizeof(void*);
        case Prism::ScriptFieldType::MeshRef:
        case Prism::ScriptFieldType::MaterialRef:
        case Prism::ScriptFieldType::Texture2DRef:
            return sizeof(void*);
        default: PR_CORE_ASSERT(false, "未注册的类型");
        }

        return 0;
    }

    struct PRISM_API ScriptFieldMetadata
    {
        std::string Name;
        ScriptFieldType Type = ScriptFieldType::None;
        Buffer DefaultValue;
        Rolky::Type* ManagedType = nullptr;
    };

    enum class LifecycleMethod : uint16_t
    {
        Awake           = BIT(0),
        OnEnable        = BIT(1),
        OnDisable       = BIT(2),
        OnCreate        = BIT(3),
        OnUpdate        = BIT(4),
        LateUpdate      = BIT(5),
        OnFixedUpdate   = BIT(6),
        OnDestroy       = BIT(7),
        OnCollisionBegin = BIT(8),
        OnCollisionEnd  = BIT(9),
        OnTriggerBegin  = BIT(10),
        OnTriggerEnd    = BIT(11),
    };

    struct PRISM_API ScriptClassMetadata
    {
        UUID ClassID;
        std::string FullName;
        std::string ModuleName;
        std::string ClassName;
        uint16_t LifecycleMask = 0;
        std::unordered_map<uint32_t, ScriptFieldMetadata> Fields;  // key = FNV hash of field name
    };

    // ── Python 标注 → ScriptFieldType 映射 ──
    PRISM_API ScriptFieldType GetFieldTypeFromPythonAnnotation(const std::string& annotation);

}
