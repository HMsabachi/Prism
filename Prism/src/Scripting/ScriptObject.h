#pragma once
#include "Prism/Core/Core.h"
#include "Utility/ScriptType.h"
#include <cstdint>
#include <cstring>
#include <memory>

namespace Prism {

class ScriptObject {
public:
    virtual ~ScriptObject() = default;
    virtual bool IsValid() const = 0;
    virtual bool TryInvokeMethodInternal(std::string_view InMethodName, const void** InParameters, const ScriptType* InParameterTypes, size_t InLength) const = 0;
    virtual bool TryInvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ScriptType* InParameterTypes, size_t InLength, void* InResultStorage) const = 0;
    virtual void InvokeMethodInternal(std::string_view InMethodName, const void** InParameters, const ScriptType* InParameterTypes, size_t InLength) const = 0;
    virtual void InvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ScriptType* InParameterTypes, size_t InLength, void* InResultStorage) const = 0;

public:
    template<typename TReturn, typename... TArgs>
    TReturn InvokeMethod(std::string_view InMethodName, TArgs&&... InParameters) const
    {
        constexpr size_t parameterCount = sizeof...(InParameters);

        TReturn result;

        if constexpr (parameterCount > 0)
        {
            const void* parameterValues[parameterCount];
            ScriptType parameterTypes[parameterCount];
            AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
            InvokeMethodRetInternal(InMethodName, parameterValues, parameterTypes, parameterCount, &result);
        }
        else
        {
            InvokeMethodRetInternal(InMethodName, nullptr, nullptr, 0, &result);
        }

        return result;
    }

    template<typename... TArgs>
    void InvokeMethod(std::string_view InMethodName, TArgs&&... InParameters) const
    {
        constexpr size_t parameterCount = sizeof...(InParameters);

        if constexpr (parameterCount > 0)
        {
            const void* parameterValues[parameterCount];
            ScriptType parameterTypes[parameterCount];
            AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
            InvokeMethodInternal(InMethodName, parameterValues, parameterTypes, parameterCount);
        }
        else
        {
            InvokeMethodInternal(InMethodName, nullptr, nullptr, 0);
        }
    }

    template<typename TReturn, typename... TArgs>
    bool TryInvokeMethod(std::string_view InMethodName, TReturn& OutResult, TArgs&&... InParameters) const
    {
        constexpr size_t parameterCount = sizeof...(InParameters);

        if constexpr (parameterCount > 0)
        {
            const void* parameterValues[parameterCount];
            ScriptType parameterTypes[parameterCount];
            AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
            return TryInvokeMethodRetInternal(InMethodName, parameterValues, parameterTypes, parameterCount, &OutResult);
        }
        else
        {
            return TryInvokeMethodRetInternal(InMethodName, nullptr, nullptr, 0, &OutResult);
        }
    }

    template<typename... TArgs>
    bool TryInvokeMethod(std::string_view InMethodName, TArgs&&... InParameters) const
    {
        constexpr size_t parameterCount = sizeof...(InParameters);

        if constexpr (parameterCount > 0)
        {
            const void* parameterValues[parameterCount];
            ScriptType parameterTypes[parameterCount];
            AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount> {});
            return TryInvokeMethodInternal(InMethodName, parameterValues, parameterTypes, parameterCount);
        }
        else
        {
            return TryInvokeMethodInternal(InMethodName, nullptr, nullptr, 0);
        }
    }

};

} // namespace Prism
