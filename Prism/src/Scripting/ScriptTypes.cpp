#include "prpch.h"
#include "ScriptTypes.h"

namespace Prism
{

    ScriptFieldType GetFieldTypeFromPythonAnnotation(const std::string& annotation)
    {
        if (annotation == "float")              return ScriptFieldType::Float;
        if (annotation == "double")             return ScriptFieldType::Double;
        if (annotation == "int")                return ScriptFieldType::Int32;
        if (annotation == "int8")               return ScriptFieldType::Int8;
        if (annotation == "int16")              return ScriptFieldType::Int16;
        if (annotation == "int32")              return ScriptFieldType::Int32;
        if (annotation == "int64")              return ScriptFieldType::Int64;
        if (annotation == "uint8")              return ScriptFieldType::UInt8;
        if (annotation == "uint16")             return ScriptFieldType::UInt16;
        if (annotation == "uint32")             return ScriptFieldType::UInt32;
        if (annotation == "uint64")             return ScriptFieldType::UInt64;
        if (annotation == "bool")               return ScriptFieldType::Bool;
        if (annotation == "Vector2" || annotation == "Prism.Vector2")  return ScriptFieldType::Vector2;
        if (annotation == "Vector3" || annotation == "Prism.Vector3")  return ScriptFieldType::Vector3;
        if (annotation == "Vector4" || annotation == "Prism.Vector4")  return ScriptFieldType::Vector4;
        return ScriptFieldType::None;
    }

}
