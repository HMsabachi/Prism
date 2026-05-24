#include "prpch.h"
#include "ScriptTypes.h"

namespace Prism
{

    ScriptFieldType GetFieldTypeFromPythonAnnotation(const std::string& annotation)
    {
        if (annotation == "float")              return ScriptFieldType::Float;
        if (annotation == "int")                return ScriptFieldType::Int32;
        if (annotation == "bool")               return ScriptFieldType::Bool;
        if (annotation == "str")                return ScriptFieldType::String;
        if (annotation == "Vector2" || annotation == "Prism.Vector2")  return ScriptFieldType::Vector2;
        if (annotation == "Vector3" || annotation == "Prism.Vector3")  return ScriptFieldType::Vector3;
        if (annotation == "Vector4" || annotation == "Prism.Vector4")  return ScriptFieldType::Vector4;
        return ScriptFieldType::None;
    }

}
