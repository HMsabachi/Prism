#pragma once
#include <type_traits>
#include <cstring>

namespace Prism
{
    class Variant
    {
    public:
        static constexpr size_t MaxSize = 64;
        Variant() : m_Size(0), m_Buffer{} {}
        template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Variant>>>
        Variant(const T& value){ Set(value); }
        Variant(const Variant&) = default;
        Variant& operator=(const Variant&) = default;
        Variant(Variant&&) noexcept = default;
        Variant& operator=(Variant&&) noexcept = default;
        ~Variant() = default;
        template <typename T>
        void Set(const T& value)
        {
            using DecayedT = std::decay_t<T>;
            static_assert(sizeof(DecayedT) <= MaxSize, "超过64字节限制");
            static_assert(std::is_trivially_copyable_v<DecayedT>, "只支持存储平凡类型(Trivially Copyable)");
            m_Size = sizeof(DecayedT);
            std::memcpy(m_Buffer, &value, m_Size); 
        }
        template <typename T>
        T& Get() { return *reinterpret_cast<T*>(m_Buffer); }
        template <typename T>
        const T& Get() const { return *reinterpret_cast<const T*>(m_Buffer); }
        size_t GetSize() const { return m_Size; }
        void Reset() { m_Size = 0; }
        bool HasValue() const { return m_Size > 0; }
    private:
        alignas(std::max_align_t) uint8_t m_Buffer[MaxSize];
        size_t m_Size = 0;
    };
}
