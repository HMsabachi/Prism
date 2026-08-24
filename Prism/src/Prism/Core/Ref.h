#pragma once

#include <stdint.h>
#include <atomic>
#include <type_traits>
#include <utility>
#include <cstddef>

#include "Core.h"

namespace Prism
{
    namespace RefUtils
    {
        void PRISM_API AddToLiveReferences(void* instance);
        void PRISM_API RemoveFromLiveReferences(void* instance);
        bool PRISM_API IsLive(void* instance);
        size_t PRISM_API GetLiveReferenceCount();
    }

    class PRISM_API RefCounted
    {
    public:
        RefCounted()
        {
            RefUtils::AddToLiveReferences(this);
        }

        virtual ~RefCounted()
        {
            RefUtils::RemoveFromLiveReferences(this);
        }

        void IncRefCount() const
        {
            ++m_RefCount;
        }

        void DecRefCount() const
        {
            --m_RefCount;
        }

        uint32_t GetRefCount() const { return m_RefCount; }

    private:
        mutable std::atomic<uint32_t> m_RefCount{ 0 };
    };

    template<typename T>
    class Ref
    {
    public:
        Ref()
            : m_Instance(nullptr)
        {
        }

        Ref(std::nullptr_t)
            : m_Instance(nullptr)
        {
        }

        Ref(T* instance)
            : m_Instance(instance)
        {
            static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");

            IncRef();
        }

        template<typename T2>
        Ref(const Ref<T2>& other)
            : m_Instance((T*)other.m_Instance)
        {
            IncRef();
        }

        template<typename T2>
        Ref(Ref<T2>&& other)
            : m_Instance((T*)other.m_Instance)
        {
            other.m_Instance = nullptr;
        }

        Ref(const Ref<T>& other)
            : m_Instance(other.m_Instance)
        {
            IncRef();
        }

        ~Ref()
        {
            DecRef();
        }

        static Ref<T> CopyWithoutIncrement(const Ref<T>& other)
        {
            Ref<T> result = nullptr;
            result.m_Instance = other.m_Instance;
            return result;
        }

        Ref& operator=(std::nullptr_t)
        {
            DecRef();
            m_Instance = nullptr;
            return *this;
        }

        Ref& operator=(const Ref<T>& other)
        {
            if (this == &other)
                return *this;

            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }

        template<typename T2>
        Ref& operator=(const Ref<T2>& other)
        {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }

        template<typename T2>
        Ref& operator=(Ref<T2>&& other)
        {
            DecRef();

            m_Instance = other.m_Instance;
            other.m_Instance = nullptr;
            return *this;
        }

        operator bool() { return m_Instance != nullptr; }
        operator bool() const { return m_Instance != nullptr; }

        T* operator->() { return m_Instance; }
        const T* operator->() const { return m_Instance; }

        T& operator*() { return *m_Instance; }
        const T& operator*() const { return *m_Instance; }

        T* Raw() { return  m_Instance; }
        const T* Raw() const { return  m_Instance; }

        void Reset(T* instance = nullptr)
        {
            if (m_Instance == instance) return;
            DecRef();
            m_Instance = instance;
            if (m_Instance)
                IncRef();
        }

        template<typename T2>
        Ref<T2> As() const
        {
            return Ref<T2>(*this);
        }

        template<typename... Args>
        static Ref<T> Create(Args&&... args)
        {
#if PR_TRACK_MEMORY && defined(PR_PLATFORM_WINDOWS)
            return Ref<T>(new(typeid(T).name()) T(std::forward<Args>(args)...));
#else
            return Ref<T>(new T(std::forward<Args>(args)...));
#endif
        }

        bool operator==(const Ref<T>& other) const
        {
            return m_Instance == other.m_Instance;
        }

        bool operator!=(const Ref<T>& other) const
        {
            return !(*this == other);
        }

        bool EqualsObject(const Ref<T>& other)
        {
            if (!m_Instance || !other.m_Instance)
                return false;

            return *m_Instance == *other.m_Instance;
        }

    private:
        void IncRef() const { if (m_Instance) ((RefCounted*)m_Instance)->IncRefCount(); }

        void DecRef() const
        {
            if (!m_Instance) return;
            RefCounted* instance = (RefCounted*)m_Instance;
            instance->DecRefCount();
            if (instance->GetRefCount() == 0)
            {
                delete instance;
                m_Instance = nullptr;
            }
        }

        template<class T2>
        friend class Ref;
        mutable T* m_Instance;
    };

    template<typename T>
    class WeakRef
    {
    public:
        WeakRef() = default;

        WeakRef(Ref<T> ref)
            : m_Instance(ref.Raw()){}
        WeakRef(T* instance)
            : m_Instance(instance){}

        T* operator->() { return m_Instance; }
        const T* operator->() const { return m_Instance; }

        T& operator*() { return *m_Instance; }
        const T& operator*() const { return *m_Instance; }

        bool IsValid() const { return m_Instance ? RefUtils::IsLive(m_Instance) : false; }
        operator bool() const { return IsValid(); }

        T* Raw() { return  m_Instance; }
        const T* Raw() const { return  m_Instance; }

        template<typename T2>
        WeakRef<T2> As() const
        {
            return WeakRef<T2>(dynamic_cast<T2*>(m_Instance));
        }
    private:
        T* m_Instance = nullptr;
    };
}

namespace std
{
    template<typename T>
    struct hash<Prism::Ref<T>>
    {
        size_t operator()(const Prism::Ref<T>& res) const noexcept
        {
            return std::hash<const void*>()((const void*)res.Raw());
        }
    };
}
