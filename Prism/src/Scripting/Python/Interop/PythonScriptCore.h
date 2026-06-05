#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "PythonMathBridge.h"

namespace Prism::Python {
    struct ScriptValue;

    class GILGuard
    {
    public:
        GILGuard();
        ~GILGuard();
        GILGuard(const GILGuard&) = delete;
        GILGuard& operator=(const GILGuard&) = delete;
    private:
        void* m_State;
    };

    class PyErrorSaver
    {
    public:
        PyErrorSaver();
        ~PyErrorSaver();
        void Clear();
        void Log();
    private:
        void Restore();
        void* m_Exc;
        void* m_Val;
        void* m_Tb;
    };

    // RAII 包装
    class ScriptRef
    {
    public:
        ScriptRef() = default;
        ~ScriptRef();

        explicit ScriptRef(ScriptValue* value);
        ScriptRef(const ScriptRef& other);
        ScriptRef(ScriptRef&& other) noexcept;
        ScriptRef& operator=(const ScriptRef& other);
        ScriptRef& operator=(ScriptRef&& other) noexcept;

        bool IsValid() const { return m_Value != nullptr; }
        bool IsNone() const;

        static ScriptRef Adopt(ScriptValue* value);
        ScriptValue* Detach();
        ScriptValue* Get() const { return m_Value; }

        ScriptRef GetAttribute(const char* name) const;
        void SetAttribute(const char* name, const ScriptRef& value) const;
        bool HasAttribute(const char* name) const;

    private:
        ScriptValue* m_Value = nullptr;
        friend class ScriptModule;
        friend class ScriptClass;
        friend class ScriptObject;
        friend class NativeModule;
    };

    class ScriptHost
    {
    public:
        static bool Initialize();
        static void Shutdown();
        static bool IsInitialized();
    };

    class ScriptModule
    {
    public:
        ScriptModule() = default;
        static ScriptModule Import(const char* name);
        static bool ModuleExists(const char* name);

        ScriptRef GetAttribute(const char* name) const;
        bool HasAttribute(const char* name) const;
        std::vector<std::string> GetNames() const;

        bool IsValid() const { return m_Ref.IsValid(); }

    private:
        friend class ScriptClass;
        explicit ScriptModule(ScriptRef ref) : m_Ref(std::move(ref)) {}
        ScriptRef m_Ref;
    };

    class ScriptClass
    {
    public:
        ScriptClass() = default;
        static ScriptClass From(const ScriptModule& mod, const char* name);

        // 从已有 ScriptRef（PyObject*）构造，用于 wrapper 中接收 Python 传过来的类对象
        static ScriptClass FromRef(const ScriptRef& ref);

        // 获取 Python 类型唯一 ID（与 id(cls) 等效，封装在内部）
        uint64_t GetTypeId() const;

        std::string GetName() const;
        std::string GetModuleName() const;
        std::string GetFullName() const;
        bool IsSubclassOf(const ScriptClass& other) const;

        // ── 字段反射 ──
        struct FieldInfo
        {
            std::string Name;
            std::string TypeAnnotation;
            bool HasDefault = false;
        };
        std::vector<FieldInfo> GetFields() const;

        bool HasMethod(const char* name) const;
        bool HasMethodWithArity(const char* name, int userArgCount) const;

        using AnnotationMap = std::unordered_map<std::string, ScriptRef>;
        AnnotationMap GetAnnotations() const;

        ScriptObject CreateInstance() const;

        template<typename... TArgs>
        ScriptObject CreateInstance(TArgs&&... args) const;

        bool IsValid() const { return m_Ref.IsValid(); }

    private:
        friend class ScriptObject;
        explicit ScriptClass(ScriptRef ref) : m_Ref(std::move(ref)) {}
        ScriptRef m_Ref;
        ScriptObject CreateInstanceInternal(const ScriptRef& tuple) const;
    };

    class ScriptObject
    {
    public:
        ScriptObject() = default;
        explicit ScriptObject(ScriptRef ref);

        ScriptRef GetAttribute(const char* name) const { return m_Ref.GetAttribute(name); }
        void SetAttribute(const char* name, const ScriptRef& value) const { m_Ref.SetAttribute(name, value); }
        bool HasAttribute(const char* name) const { return m_Ref.HasAttribute(name); }

        template<typename TReturn = void, typename... TArgs>
        TReturn Invoke(const char* method, TArgs&&... args)
        {
            if constexpr (std::is_same_v<TReturn, void>)
                InvokeArgs(method, std::forward<TArgs>(args)...);
            else
            {
                ScriptRef result = InvokeArgs(method, std::forward<TArgs>(args)...);
                return ConvertFromScriptRef<TReturn>(result);
            }
        }

        // GetField<T> — 支持标量和 glm::vec2/3/4
        template<typename T>
        T GetField(const char* name) const;
        template<typename T>
        void SetField(const char* name, T value);

        void GetFieldRaw(const char* name, void* buffer) const;
        void SetFieldRaw(const char* name, const void* buffer) const;

        bool IsValid() const { return m_Ref.IsValid(); }

        ScriptRef GetRef() const { return m_Ref; }

        template<typename... TArgs>
        ScriptRef InvokeArgs(const char* method, TArgs&&... args);
        ScriptRef InvokeWithTuple(const char* method, const ScriptRef& tuple);

    private:
        friend class ScriptClass;

        ScriptRef m_Ref;
    };

    using NativeFunction = ScriptValue * (*)(ScriptValue* self, ScriptValue* args);

    class NativeModule
    {
    public:
        explicit NativeModule(const char* name);
        ~NativeModule();
        void AddFunction(const char* name, NativeFunction func, const char* doc = nullptr);
        void Register();

    private:
        struct FuncEntry
        {
            std::string Name;
            NativeFunction Func;
            std::string Doc;
        };
        std::string m_Name;
        std::vector<FuncEntry> m_Functions;
    };

    // 转换模板 — 将 C++ 类型分派到 PythonMathBridge 的转换函数
    template<typename T>
    ScriptRef ToValue(T&& value)
    {
        using Raw = std::decay_t<T>;
        if constexpr (std::is_same_v<Raw, float>)
            return FloatToValue(static_cast<float>(value));
        else if constexpr (std::is_same_v<Raw, double>)
            return FloatToValue(static_cast<float>(value));
        else if constexpr (std::is_same_v<Raw, int8_t>)
            return IntToValue(static_cast<int32_t>(value));
        else if constexpr (std::is_same_v<Raw, int16_t>)
            return IntToValue(static_cast<int32_t>(value));
        else if constexpr (std::is_same_v<Raw, int32_t>)
            return IntToValue(value);
        else if constexpr (std::is_same_v<Raw, int64_t>)
            return IntToValue(static_cast<int32_t>(value));
        else if constexpr (std::is_same_v<Raw, uint8_t>)
            return UInt64ToValue(static_cast<uint64_t>(value));
        else if constexpr (std::is_same_v<Raw, uint16_t>)
            return UInt64ToValue(static_cast<uint64_t>(value));
        else if constexpr (std::is_same_v<Raw, uint32_t>)
            return UInt64ToValue(value);
        else if constexpr (std::is_same_v<Raw, uint64_t>)
            return UInt64ToValue(value);
        else if constexpr (std::is_same_v<Raw, std::string>)
            return StringToValue(value);
        else if constexpr (std::is_same_v<Raw, const char*>)
            return StringToValue(std::string_view(value));
        else if constexpr (std::is_same_v<Raw, bool>)
            return BoolToValue(value);
        else if constexpr (std::is_same_v<Raw, glm::vec2>)
            return Vec2ToValue(value);
        else if constexpr (std::is_same_v<Raw, glm::vec3>)
            return Vec3ToValue(value);
        else if constexpr (std::is_same_v<Raw, glm::vec4>)
            return Vec4ToValue(value);
        else
            static_assert(false, "Unsupported type for Python conversion");
    }

    template<typename T>
    T ConvertFromScriptRef(const ScriptRef& v)
    {
        using Raw = std::decay_t<T>;
        if constexpr (std::is_same_v<Raw, float>)
            return ValueToFloat(v);
        else if constexpr (std::is_same_v<Raw, double>)
            return static_cast<double>(ValueToFloat(v));
        else if constexpr (std::is_same_v<Raw, int8_t>)
            return static_cast<int8_t>(ValueToInt(v));
        else if constexpr (std::is_same_v<Raw, int16_t>)
            return static_cast<int16_t>(ValueToInt(v));
        else if constexpr (std::is_same_v<Raw, int32_t>)
            return ValueToInt(v);
        else if constexpr (std::is_same_v<Raw, int64_t>)
            return static_cast<int64_t>(ValueToInt(v));
        else if constexpr (std::is_same_v<Raw, uint8_t>)
            return static_cast<uint8_t>(ValueToUInt64(v));
        else if constexpr (std::is_same_v<Raw, uint16_t>)
            return static_cast<uint16_t>(ValueToUInt64(v));
        else if constexpr (std::is_same_v<Raw, uint32_t>)
            return static_cast<uint32_t>(ValueToUInt64(v));
        else if constexpr (std::is_same_v<Raw, uint64_t>)
            return ValueToUInt64(v);
        else if constexpr (std::is_same_v<Raw, std::string>)
            return ValueToString(v);
        else if constexpr (std::is_same_v<Raw, bool>)
            return ValueToBool(v);
        else if constexpr (std::is_same_v<Raw, glm::vec2>)
            return ValueToVec2(v);
        else if constexpr (std::is_same_v<Raw, glm::vec3>)
            return ValueToVec3(v);
        else if constexpr (std::is_same_v<Raw, glm::vec4>)
            return ValueToVec4(v);
        else
            static_assert(false, "Unsupported type for Python return conversion");
    }

    template<typename... TArgs>
    ScriptObject ScriptClass::CreateInstance(TArgs&&... args) const
    {
        if constexpr (sizeof...(TArgs) > 0)
        {
            ScriptRef pyArgs[] = { ToValue(std::forward<TArgs>(args))... };
            ScriptRef tuple = MakeTuple(pyArgs, sizeof...(TArgs));
            return CreateInstanceInternal(tuple);
        }
        else
        {
            return CreateInstance();
        }
    }

    // ScriptObject 方法模板
    template<typename... TArgs>
    ScriptRef ScriptObject::InvokeArgs(const char* method, TArgs&&... args)
    {
        if constexpr (sizeof...(TArgs) == 0)
        {
            ScriptRef empty;
            return InvokeWithTuple(method, empty);
        }
        else
        {
            ScriptRef pyArgs[] = { ToValue(std::forward<TArgs>(args))... };
            ScriptRef tuple = MakeTuple(pyArgs, sizeof...(TArgs));
            return InvokeWithTuple(method, tuple);
        }
    }

    template<typename T>
    T ScriptObject::GetField(const char* name) const
    {
        using Raw = std::decay_t<T>;
        ScriptRef val = GetAttribute(name);

        if constexpr (std::is_same_v<Raw, float>)
            return static_cast<T>(ValueToFloat(val));
        else if constexpr (std::is_same_v<Raw, double>)
            return static_cast<T>(static_cast<double>(ValueToFloat(val)));
        else if constexpr (std::is_same_v<Raw, int8_t>)
            return static_cast<T>(static_cast<int8_t>(ValueToInt(val)));
        else if constexpr (std::is_same_v<Raw, int16_t>)
            return static_cast<T>(static_cast<int16_t>(ValueToInt(val)));
        else if constexpr (std::is_same_v<Raw, int32_t>)
            return static_cast<T>(ValueToInt(val));
        else if constexpr (std::is_same_v<Raw, int64_t>)
            return static_cast<T>(static_cast<int64_t>(ValueToInt(val)));
        else if constexpr (std::is_same_v<Raw, uint8_t>)
            return static_cast<T>(static_cast<uint8_t>(ValueToUInt64(val)));
        else if constexpr (std::is_same_v<Raw, uint16_t>)
            return static_cast<T>(static_cast<uint16_t>(ValueToUInt64(val)));
        else if constexpr (std::is_same_v<Raw, uint32_t>)
            return static_cast<T>(static_cast<uint32_t>(ValueToUInt64(val)));
        else if constexpr (std::is_same_v<Raw, uint64_t>)
            return static_cast<T>(ValueToUInt64(val));
        else if constexpr (std::is_same_v<Raw, std::string>)
            return static_cast<T>(ValueToString(val));
        else if constexpr (std::is_same_v<Raw, bool>)
            return static_cast<T>(ValueToBool(val));
        else if constexpr (std::is_same_v<Raw, glm::vec2>)
            return static_cast<T>(ValueToVec2(val));
        else if constexpr (std::is_same_v<Raw, glm::vec3>)
            return static_cast<T>(ValueToVec3(val));
        else if constexpr (std::is_same_v<Raw, glm::vec4>)
            return static_cast<T>(ValueToVec4(val));
        else
            static_assert(false, "Unsupported type for Python field access");
    }

    template<typename T>
    void ScriptObject::SetField(const char* name, T value)
    {
        SetAttribute(name, ToValue(std::forward<T>(value)));
    }

} // namespace Prism::Python


