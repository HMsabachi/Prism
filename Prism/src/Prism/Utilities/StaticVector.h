#pragma once
#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace Prism
{
    template<class T, std::size_t N>
    class StaticVector
    {
        static_assert(N > 0);

        alignas(T) std::byte m_Data[sizeof(T) * N];
        std::size_t m_Size = 0;

        constexpr T* ptr(std::size_t i) noexcept { return std::launder(reinterpret_cast<T*>(m_Data + i * sizeof(T))); }
        constexpr const T* ptr(std::size_t i) const noexcept { return std::launder(reinterpret_cast<const T*>(m_Data + i * sizeof(T))); }

        constexpr void destroy(std::size_t i) noexcept {
            if constexpr (!std::is_trivially_destructible_v<T>)
                std::destroy_at(ptr(i));
        }

        constexpr void destroy_range(std::size_t first, std::size_t last) noexcept {
            if constexpr (!std::is_trivially_destructible_v<T>)
                for (; first < last; ++first) std::destroy_at(ptr(first));
        }

    public:
        using value_type = T;
        using size_type = std::size_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = T*;
        using const_reverse_iterator = const T*;

        constexpr StaticVector() noexcept = default;

        template<std::size_t OtherN>
        constexpr StaticVector(const StaticVector<T, OtherN>& other)
            requires std::is_copy_constructible_v<T>
        {
            assert(other.size() <= N);
            for (const T& v : other) emplace_back(v);
        }

        template<std::size_t OtherN>
        constexpr StaticVector(StaticVector<T, OtherN>&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
            requires std::is_move_constructible_v<T>
        {
            assert(other.size() <= N);
            for (T& v : other) emplace_back(std::move(v));
            other.clear();
        }

        constexpr ~StaticVector() { clear(); }

        template<std::size_t OtherN>
        constexpr StaticVector& operator=(const StaticVector<T, OtherN>& other)
            requires std::is_copy_constructible_v<T>
        {
            assert(other.size() <= N);
            if (this != reinterpret_cast<StaticVector*>(&other)) {
                clear();
                for (const T& v : other) emplace_back(v);
            }
            return *this;
        }

        template<std::size_t OtherN>
        constexpr StaticVector& operator=(StaticVector<T, OtherN>&& other)
            noexcept(std::is_nothrow_move_constructible_v<T>)
            requires std::is_move_constructible_v<T>
        {
            assert(other.size() <= N);
            if (this != reinterpret_cast<StaticVector*>(&other)) {
                clear();
                for (T& v : other) emplace_back(std::move(v));
                other.clear();
            }
            return *this;
        }

        template<class... Args>
        constexpr reference emplace_back(Args&&... args) { assert(m_Size < N); T* p = ptr(m_Size++); return *std::construct_at(p, std::forward<Args>(args)...);}

        constexpr void push_back(const T& v) requires std::is_copy_constructible_v<T> { emplace_back(v); }
        constexpr void push_back(T&& v) requires std::is_move_constructible_v<T> { emplace_back(std::move(v)); }
        constexpr void pop_back() noexcept { assert(m_Size); destroy(--m_Size); }
        constexpr void clear() noexcept { destroy_range(0, m_Size); m_Size = 0; }

        constexpr void resize(std::size_t n) requires std::is_default_constructible_v<T> {
            assert(n <= N);
            if (n > m_Size) while (m_Size < n) emplace_back();
            else while (m_Size > n) pop_back();
        }

        constexpr void resize(std::size_t n, const T& v) requires std::is_copy_constructible_v<T> {
            assert(n <= N);
            if (n > m_Size) while (m_Size < n) emplace_back(v);
            else while (m_Size > n) pop_back();
        }

        constexpr iterator erase(iterator pos) {
            const auto i = static_cast<std::size_t>(pos - begin());
            assert(i < m_Size);
            if (i + 1 < m_Size)
                std::move(begin() + i + 1, end(), begin() + i);
            pop_back();
            return begin() + i;
        }

        constexpr iterator erase(iterator first, iterator last) {
            const auto a = static_cast<std::size_t>(first - begin());
            const auto b = static_cast<std::size_t>(last - begin());
            assert(a <= b && b <= m_Size);
            if (a == b) return begin() + a;
            const auto count = b - a;
            std::move(begin() + b, end(), begin() + a);
            destroy_range(m_Size - count, m_Size);
            m_Size -= count;
            return begin() + a;
        }

        template<std::size_t OtherN>
        [[nodiscard]] constexpr bool operator==(const StaticVector<T, OtherN>& other) const noexcept {
            if (m_Size != other.m_Size) return false;
            for (std::size_t i = 0; i < m_Size; ++i)
                if (!(*ptr(i) == *other.ptr(i))) return false;
            return true;
        }
        template<std::size_t OtherN>
        [[nodiscard]] constexpr bool operator!=(const StaticVector<T, OtherN>& other) const noexcept { return !(*this == other); }

        [[nodiscard]] constexpr reference operator[](std::size_t i) noexcept { assert(i < m_Size); return *ptr(i); }
        [[nodiscard]] constexpr const_reference operator[](std::size_t i) const noexcept { assert(i < m_Size); return *ptr(i); }

        [[nodiscard]] constexpr reference front() noexcept { assert(m_Size); return *ptr(0);}
        [[nodiscard]] constexpr const_reference front() const noexcept { assert(m_Size); return *ptr(0);}
        [[nodiscard]] constexpr reference back() noexcept { assert(m_Size); return *ptr(m_Size - 1); }
        [[nodiscard]] constexpr const_reference back() const noexcept { assert(m_Size); return *ptr(m_Size - 1); }

        [[nodiscard]] constexpr pointer data() noexcept { return ptr(0);}
        [[nodiscard]] constexpr const_pointer data() const noexcept { return ptr(0);}

        [[nodiscard]] constexpr iterator begin() noexcept { return data(); }
        [[nodiscard]] constexpr const_iterator begin() const noexcept { return data(); }
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return end(); }
        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return end(); }
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        [[nodiscard]] constexpr iterator end() noexcept { return data() + m_Size; }
        [[nodiscard]] constexpr const_iterator end() const noexcept { return data() + m_Size; }
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }
        [[nodiscard]] constexpr reverse_iterator rend() noexcept { return begin(); }
        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return begin(); }
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

        [[nodiscard]] constexpr std::size_t size() const noexcept { return m_Size; }
        [[nodiscard]] constexpr std::size_t capacity() const noexcept { return N; }
        [[nodiscard]] constexpr std::size_t max_size() const noexcept { return N; }
        [[nodiscard]] constexpr std::size_t remaining() const noexcept { return N - m_Size; }
        [[nodiscard]] constexpr bool empty() const noexcept { return m_Size == 0; }
        [[nodiscard]] constexpr bool full() const noexcept { return m_Size == N; }

        template<std::size_t OtherN>
        constexpr void swap(StaticVector<T, OtherN>& other) noexcept(std::is_nothrow_swappable_v<T>) {
            if (this == reinterpret_cast<StaticVector*>(&other)) return;
            assert(other.size() <= N && m_Size <= OtherN);
            const auto common = std::min(m_Size, other.m_Size);
            for (std::size_t i = 0; i < common; ++i)
                std::swap((*this)[i], other[i]);
            if (m_Size < other.m_Size) {
                for (std::size_t i = common; i < other.m_Size; ++i)
                    emplace_back(std::move(other[i]));
                other.destroy_range(common, other.m_Size);
                other.m_Size = common;
            }
            else if (m_Size > other.m_Size) {
                for (std::size_t i = common; i < m_Size; ++i)
                    other.emplace_back(std::move((*this)[i]));
                destroy_range(common, m_Size);
                m_Size = common;
            }
        }


    };
    template<class T, std::size_t N1, std::size_t N2>
    void swap(StaticVector<T, N1>& a, StaticVector<T, N2>& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
}
