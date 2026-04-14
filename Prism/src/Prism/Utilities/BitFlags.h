#pragma once
#include <cstdint>
#include <type_traits>

// ====================== 位标志模板 ======================

/**
 * @brief 位标志模板类，让 enum class 自动支持位运算
 * @tparam Enum 必须是 enum class 类型，且指定了底层整数类型（如 : uint32_t）
 */
template<typename Enum>
class BitFlags
{
    static_assert(std::is_enum_v<Enum>, "BitFlags 只能用于 enum class 类型");

public:
    using Type = Enum;                                    // 枚举类型
    using UnderlyingType = std::underlying_type_t<Enum>;  // 底层整数类型

private:
    UnderlyingType m_value = 0;   // 实际存储的值

public:
    // 默认构造
    constexpr BitFlags() = default;

    // 从单个标志构造
    constexpr BitFlags(Enum flag) : m_value(static_cast<UnderlyingType>(flag)) {}

    // 从多个标志构造（使用 | 运算）
    constexpr BitFlags(std::initializer_list<Enum> flags)
    {
        for (auto f : flags)
            m_value |= static_cast<UnderlyingType>(f);
    }

    // 检查是否包含某个标志
    constexpr bool Has(Enum flag) const
    {
        return (m_value & static_cast<UnderlyingType>(flag)) != 0;
    }

    // 添加标志
    constexpr BitFlags& Set(Enum flag)
    {
        m_value |= static_cast<UnderlyingType>(flag);
        return *this;
    }

    // 移除标志
    constexpr BitFlags& Reset(Enum flag)
    {
        m_value &= ~static_cast<UnderlyingType>(flag);
        return *this;
    }

    // 切换标志（有则移除，无则添加）
    constexpr BitFlags& Flip(Enum flag)
    {
        m_value ^= static_cast<UnderlyingType>(flag);
        return *this;
    }

    // 清空所有标志
    constexpr void Clear() { m_value = 0; }

    // 获取原始底层值（供 OpenGL 等底层 API 使用）
    constexpr UnderlyingType Value() const { return m_value; }

    // 隐式转换为 bool（判断是否有任何标志）
    constexpr explicit operator bool() const { return m_value != 0; }

    // 位运算符重载（方便使用）
    constexpr BitFlags operator|(Enum flag) const { return BitFlags(*this) |= flag; }
    constexpr BitFlags operator&(Enum flag) const { return BitFlags(*this) &= flag; }
    constexpr BitFlags operator^(Enum flag) const { return BitFlags(*this) ^= flag; }

    constexpr BitFlags& operator|=(Enum flag) { Set(flag); return *this; }
    constexpr BitFlags& operator&=(Enum flag) { Reset(~flag); return *this; }  // 注意：这里用 Reset(~flag) 实现 &= 
    constexpr BitFlags& operator^=(Enum flag) { Flip(flag); return *this; }
};

// ====================== 全局辅助函数（可选，方便直接使用） ======================

template<typename Enum>
constexpr bool HasFlag(Enum flags, Enum flagToCheck)
{
    using UT = std::underlying_type_t<Enum>;
    return (static_cast<UT>(flags) & static_cast<UT>(flagToCheck)) != 0;
}

template<typename Enum>
constexpr Enum operator|(Enum a, Enum b)
{
    using UT = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<UT>(a) | static_cast<UT>(b));
}

template<typename Enum>
constexpr Enum operator&(Enum a, Enum b)
{
    using UT = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<UT>(a) & static_cast<UT>(b));
}

template<typename Enum>
constexpr Enum operator~(Enum a)
{
    using UT = std::underlying_type_t<Enum>;
    return static_cast<Enum>(~static_cast<UT>(a));
}

template<typename Enum>
constexpr Enum& operator|=(Enum& a, Enum b) { a = a | b; return a; }

template<typename Enum>
constexpr Enum& operator&=(Enum& a, Enum b) { a = a & b; return a; }