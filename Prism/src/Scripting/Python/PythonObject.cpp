#include "prpch.h"
#include "PythonObject.h"
#include <vector>

namespace Prism {

void PythonObject::InvokeMethodInternal(std::string_view name, const void** params, const ScriptType* types, size_t length) const
{
    if (!m_Object.IsValid())
        return;

    if (length == 0)
    {
        m_Object.Invoke(name.data());
        return;
    }

    // 将类型擦除的参数转为 ScriptRef 数组
    std::vector<Python::ScriptRef> refs;
    refs.reserve(length);
    for (size_t i = 0; i < length; i++)
    {
        switch (types[i])
        {
        case ScriptType::Float:
            refs.push_back(Python::FloatToValue(*static_cast<const float*>(params[i])));
            break;
        case ScriptType::Double:
            refs.push_back(Python::FloatToValue(static_cast<float>(*static_cast<const double*>(params[i]))));
            break;
        case ScriptType::SByte:
            refs.push_back(Python::IntToValue(static_cast<int32_t>(*static_cast<const int8_t*>(params[i]))));
            break;
        case ScriptType::Short:
            refs.push_back(Python::IntToValue(static_cast<int32_t>(*static_cast<const int16_t*>(params[i]))));
            break;
        case ScriptType::Int:
            refs.push_back(Python::IntToValue(*static_cast<const int32_t*>(params[i])));
            break;
        case ScriptType::Long:
            refs.push_back(Python::IntToValue(static_cast<int32_t>(*static_cast<const int64_t*>(params[i]))));
            break;
        case ScriptType::Byte:
            refs.push_back(Python::UInt64ToValue(static_cast<uint64_t>(*static_cast<const uint8_t*>(params[i]))));
            break;
        case ScriptType::UShort:
            refs.push_back(Python::UInt64ToValue(static_cast<uint64_t>(*static_cast<const uint16_t*>(params[i]))));
            break;
        case ScriptType::UInt:
            refs.push_back(Python::UInt64ToValue(static_cast<uint64_t>(*static_cast<const uint32_t*>(params[i]))));
            break;
        case ScriptType::ULong:
            refs.push_back(Python::UInt64ToValue(*static_cast<const uint64_t*>(params[i])));
            break;
        case ScriptType::Bool:
            refs.push_back(Python::BoolToValue(*static_cast<const bool*>(params[i])));
            break;
        case ScriptType::String:
            refs.push_back(Python::StringToValue(*static_cast<const std::string*>(params[i])));
            break;
        default:
            refs.push_back(Python::NoneValue());
            break;
        }
    }

    Python::ScriptRef tuple = Python::MakeTuple(refs.data(), static_cast<uint32_t>(length));
    m_Object.InvokeWithTuple(name.data(), tuple);
}

bool PythonObject::TryInvokeMethodInternal(std::string_view name, const void** params, const ScriptType* types, size_t length) const
{
    if (!m_Object.IsValid())
        return false;

    // 先检查方法是否存在
    if (!m_Object.HasAttribute(name.data()))
        return false;

    InvokeMethodInternal(name, params, types, length);
    return true;
}

void PythonObject::InvokeMethodRetInternal(std::string_view name, const void** params, const ScriptType* types, size_t length, void* result) const
{
    if (!m_Object.IsValid())
        return;

    if (length == 0)
    {
        Python::ScriptRef ret = m_Object.InvokeArgs(name.data());
        if (!result || ret.IsNone())
            return;
        // 默认按 float 转换；如需精确类型匹配需在 Internal 方法签名中增加 ScriptType returnType
        float f = Python::ValueToFloat(ret);
        *static_cast<float*>(result) = f;
        return;
    }

    // N 参数 + 返回值：通过 tuple 调用
    std::vector<Python::ScriptRef> refs;
    refs.reserve(length);
    for (size_t i = 0; i < length; i++)
    {
        switch (types[i])
        {
        case ScriptType::Float:   refs.push_back(Python::FloatToValue(*static_cast<const float*>(params[i]))); break;
        case ScriptType::Double:  refs.push_back(Python::FloatToValue(static_cast<float>(*static_cast<const double*>(params[i])))); break;
        case ScriptType::SByte:   refs.push_back(Python::IntToValue(static_cast<int32_t>(*static_cast<const int8_t*>(params[i])))); break;
        case ScriptType::Short:   refs.push_back(Python::IntToValue(static_cast<int32_t>(*static_cast<const int16_t*>(params[i])))); break;
        case ScriptType::Int:     refs.push_back(Python::IntToValue(*static_cast<const int32_t*>(params[i]))); break;
        case ScriptType::Long:    refs.push_back(Python::IntToValue(static_cast<int32_t>(*static_cast<const int64_t*>(params[i])))); break;
        case ScriptType::Byte:    refs.push_back(Python::UInt64ToValue(static_cast<uint64_t>(*static_cast<const uint8_t*>(params[i])))); break;
        case ScriptType::UShort:  refs.push_back(Python::UInt64ToValue(static_cast<uint64_t>(*static_cast<const uint16_t*>(params[i])))); break;
        case ScriptType::UInt:    refs.push_back(Python::UInt64ToValue(static_cast<uint64_t>(*static_cast<const uint32_t*>(params[i])))); break;
        case ScriptType::ULong:   refs.push_back(Python::UInt64ToValue(*static_cast<const uint64_t*>(params[i]))); break;
        case ScriptType::Bool:    refs.push_back(Python::BoolToValue(*static_cast<const bool*>(params[i]))); break;
        case ScriptType::String:  refs.push_back(Python::StringToValue(*static_cast<const std::string*>(params[i]))); break;
        default:                  refs.push_back(Python::NoneValue()); break;
        }
    }

    Python::ScriptRef tuple = Python::MakeTuple(refs.data(), static_cast<uint32_t>(length));
    Python::ScriptRef ret = m_Object.InvokeWithTuple(name.data(), tuple);

    if (!result || ret.IsNone())
        return;

    // 尝试按常用类型转换
    float f = Python::ValueToFloat(ret);
    *static_cast<float*>(result) = f;
}

bool PythonObject::TryInvokeMethodRetInternal(std::string_view name, const void** params, const ScriptType* types, size_t length, void* result) const
{
    if (!m_Object.IsValid())
        return false;

    if (!m_Object.HasAttribute(name.data()))
        return false;

    InvokeMethodRetInternal(name, params, types, length, result);
    return true;
}

} // namespace Prism
