#pragma once
#include <utility>
namespace Prism
{
    enum class ScriptType
    {
        Unknown,

        SByte,
        Byte,
        Short,
        UShort,
        Int,
        UInt,
        Long,
        ULong,

        Float,
        Double,

        Bool,

        String,

        Pointer,
    };

    template<typename TArg>
    constexpr ScriptType GetManagedType()
    {
        if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
            return ScriptType::Pointer;
        else if constexpr (std::is_same_v<TArg, uint8_t> || std::is_same_v<TArg, std::byte>)
            return ScriptType::Byte;
        else if constexpr (std::is_same_v<TArg, uint16_t>)
            return ScriptType::UShort;
        else if constexpr (std::is_same_v<TArg, uint32_t> || (std::is_same_v<TArg, unsigned long> && sizeof(TArg) == 4))
            return ScriptType::UInt;
        else if constexpr (std::is_same_v<TArg, uint64_t> || (std::is_same_v<TArg, unsigned long> && sizeof(TArg) == 8))
            return ScriptType::ULong;
        else if constexpr (std::is_same_v<TArg, int8_t>)
            return ScriptType::SByte;
        else if constexpr (std::is_same_v<TArg, int16_t>)
            return ScriptType::Short;
        else if constexpr (std::is_same_v<TArg, int32_t> || (std::is_same_v<TArg, long> && sizeof(TArg) == 4))
            return ScriptType::Int;
        else if constexpr (std::is_same_v<TArg, int64_t> || (std::is_same_v<TArg, long> && sizeof(TArg) == 8))
            return ScriptType::Long;
        else if constexpr (std::is_same_v<TArg, float>)
            return ScriptType::Float;
        else if constexpr (std::is_same_v<TArg, double>)
            return ScriptType::Double;
        else if constexpr (std::is_same_v<TArg, bool>)
            return ScriptType::Bool;
        else if constexpr (std::is_same_v<TArg, std::string>)
            return ScriptType::String;
        else
            return ScriptType::Unknown;
    }

    template <typename TArg, size_t TIndex>
    inline void AddToArrayI(const void** InArgumentsArr, ScriptType* InParameterTypes, TArg&& InArg)
    {
        ScriptType managedType = GetManagedType<std::remove_const_t<std::remove_reference_t<TArg>>>();
        InParameterTypes[TIndex] = managedType;

        if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
        {
            InArgumentsArr[TIndex] = reinterpret_cast<const void*>(InArg);
        }
        else
        {
            InArgumentsArr[TIndex] = reinterpret_cast<const void*>(&InArg);
        }
    }

    template <typename... TArgs, size_t... TIndices>
    inline void AddToArray(const void** InArgumentsArr, ScriptType* InParameterTypes, TArgs&&... InArgs, const std::index_sequence<TIndices...>&)
    {
        (AddToArrayI<TArgs, TIndices>(InArgumentsArr, InParameterTypes, std::forward<TArgs>(InArgs)), ...);
    }
}