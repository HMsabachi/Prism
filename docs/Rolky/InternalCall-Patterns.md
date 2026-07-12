# InternalCall 传参方法速查

> 本文档列出在 C++ 和 C# 之间通过 InternalCall 传递各种数据类型的推荐方法。  
> 前置阅读：[InternalCalls](InternalCalls) | [Interop-Types](Interop-Types)

---

## 平凡类型

| 数据类型 | C# 侧 | C++ 侧 | 说明 |
|----------|-------|--------|------|
| 整数 | `int`, `uint`, `long`, `ulong`, `short`, `byte`（按值） | `int32_t`, `uint32_t`, `uint64_t` 等（按值） | 定宽整数，确保两侧大小一致 |
| 浮点 | `float`, `double`（按值） | `float`, `double`（按值） | ABI 天然对应 |
| `bool` | `Bool32`（按值） | `Bool32`（=`uint32_t`，按值） | **一律用 `Bool32`**，避免 C++ `bool`（1 字节）与 .NET 跨边界期望 4 字节的差异 |
| 枚举 | 对应整数基类型（按值） | `int32_t` 等（按值） | 两侧底层类型一致 |
| `IntPtr` / `UIntPtr` | `IntPtr` / `UIntPtr`（按值） | `void*` / `uintptr_t`（按值） | 直接传递指针值 |
| blittable 结构体 | `T*`（指针） | `T*`（指针） | 统一用指针，Getter 写入、Setter 读取 |
| 无参 / 无返回值 | 省略 | `void` | `delegate* unmanaged[Cdecl]<void>` |

---

## 对象类型

| 数据类型 | C# 侧 | C++ 侧 | 说明 |
|----------|-------|--------|------|
| **字符串** — 输入 | `NativeString` | `String` | C# 从 `string` 隐式转换（内部分配 CoTaskMem），C++ 读 |
| **字符串** — 输出 | `NativeString`（返回值） | `String`（返回值） | C++ 调用 `String::New()` 分配，C# 用 `using` 释放 |
| **C# 托管对象** — 输出 | `NativeInstance<T>`（返回值） | `ManagedObject`（返回值） | C++ 返回 `ManagedObject`（内部含 GCHandle），C# 通过 `.Target` 取出 |
| **C# 托管对象** — 输入 | `NativeInstance<T>` | `ManagedObject` | C# 构造 `NativeInstance<T>` 传入，C++ 接收 `ManagedObject` 按值 |
| **C++ 原生对象** | `IntPtr` | `T*`（裸指针） | C++ new 对象返回裸指针作不透明句柄；C# 用完后调用析构 InternalCall |
| **非平凡 C++ 类型** — 输入 | 按值 | `Param<T>` | `Param<T>` 从栈上原始字节 memcpy，用于含构造函数的 C++ 类型 |
| **非平凡 C++ 类型** — 输出 | `T*`（指针） | `OutParam<T>` | `OutParam<T>` 通过 `reinterpret_cast` 写入 |
| **引擎 ID 对象**（Entity 等） | `UInt64`（UUID） | `uint64_t` | 只传 UUID，C++ 侧查 Map 解析 |
| **组件类型**（`typeof(T)`） | `ReflectionType`（4B 按值） | `ReflectionType` | C# 隐式转换 `typeof(T)` → 4 字节 TypeId |
| **委托 / 回调** | `IntPtr`（函数指针） | `void*` | C++ 传函数指针，C# `Marshal.GetDelegateForFunctionPointer` 还原 |
| **Blittable 数组** — 输出 | `NativeArray<T>`（返回值） | `Array<T>`（返回值） | C++ 调用 `Array<T>::New()` 分配，C# `using` 释放 |
| **Blittable 数组** — 输入 | `NativeArray<T>` | `Array<T>` | C# 从 `T[]` 构造（AllocHGlobal），C++ 读 |
| **Blittable 数组** — 零分配 | `NativeArray<T>`（Map） | `Array<T>` | C# `Map(T[])` 固定托管数组，C++ 直接读写，最后 `Unmap()` |
| **复杂嵌套结构体**（含 NativeArray 成员） | `T*`（指针） | `T*`（指针） | 含嵌套 NativeArray 的结构体很大且非平凡，必须用指针 |

---

## 关键类型定义

### Bool32

```cpp
// C++ — 4 字节 bool，值只为 0 或 1
using Bool32 = uint32_t;
static constexpr Bool32 Bool32_True  = 1;
static constexpr Bool32 Bool32_False = 0;
```

```csharp
// C# — 4 字节值类型
[StructLayout(LayoutKind.Sequential, Size = 4)]
public struct Bool32 {
    public static readonly Bool32 False = new(0);
    public static readonly Bool32 True  = new(1);
    internal uint Value;
    public static implicit operator bool(Bool32 b) => b.Value != 0;
    public static implicit operator Bool32(bool b) => new(b ? 1u : 0u);
}
```

### NativeInstance\<T\>

```csharp
[StructLayout(LayoutKind.Sequential)]
public struct NativeInstance<T> where T : class {
    internal readonly IntPtr m_Handle;
    public readonly T? Target =>
        m_Handle != IntPtr.Zero ? (T)GCHandle.FromIntPtr(m_Handle).Target : null;
    public static implicit operator bool(NativeInstance<T> i) => i.m_Handle != IntPtr.Zero;
}
```

### ManagedObject

```cpp
class ManagedObject {
    void*       m_Handle;   // GCHandle 的 IntPtr 值
    const Type* m_Type;     // 对应的类型信息
public:
    ManagedObject(const ManagedObject&);           // 拷贝 → 新 GCHandle 指向同一对象
    ManagedObject(ManagedObject&&) noexcept;       // 移动 → 转移所有权
    ~ManagedObject();                              // 析构 → GCHandle.Free()
};
```

### Param\<T\> / OutParam\<T\>

```cpp
// 输入 — 从栈上原始字节 memcpy 还原
template<std::default_initializable T>
struct Param {
    std::byte Data[sizeof(T)];
    operator T() const { T r; std::memcpy(&r, Data, sizeof(T)); return r; }
};

// 输出 — 通过指针写入
template<typename T>
struct OutParam {
    std::byte* Ptr = nullptr;
    T* operator->() noexcept { return reinterpret_cast<T*>(Ptr); }
    T& operator*()       { return *reinterpret_cast<T*>(Ptr); }
};
```

---

## 函数指针声明速查

```csharp
// 基元
delegate* unmanaged[Cdecl]<float>                                 // () → float
delegate* unmanaged[Cdecl]<int, int, int>                         // (int, int) → int

// bool — 一律 Bool32
delegate* unmanaged[Cdecl]<KeyCode, Bool32>                       // (KeyCode) → Bool32
delegate* unmanaged[Cdecl]<UInt64, Bool32, void>                  // (entity, Bool32)

// 枚举
delegate* unmanaged[Cdecl]<CameraType>                            // () → CameraType

// 结构体 Getter
delegate* unmanaged[Cdecl]<UInt64, Vector3*, void>                // (entity, out Vector3)
delegate* unmanaged[Cdecl]<UInt64, Matrix4*, void>                // (entity, out Matrix4)
delegate* unmanaged[Cdecl]<UInt64, Quaternion*, void>             // (entity, out Quat)

// 结构体 Setter
delegate* unmanaged[Cdecl]<UInt64, Vector3*, void>                // (entity, in Vector3)

// 字符串
delegate* unmanaged[Cdecl]<NativeString, UInt64>                   // (string) → entityID
delegate* unmanaged[Cdecl]<UInt64, NativeString>                   // (entity) → string

// C# 托管对象
delegate* unmanaged[Cdecl]<UInt64, NativeInstance<object>>         // (entity) → managed object
delegate* unmanaged[Cdecl]<NativeInstance<object>, void>           // (object)

// C++ 原生对象
delegate* unmanaged[Cdecl]<int, IntPtr>                            // constructor → handle
delegate* unmanaged[Cdecl]<IntPtr, void>                           // destructor
delegate* unmanaged[Cdecl]<IntPtr, float, float, float>            // (handle, x, y) → value

// 非平凡 C++ 类型
delegate* unmanaged[Cdecl]<AssetHandle, Bool32>                   // (handle) → valid
delegate* unmanaged[Cdecl]<UInt64, AssetHandle*, Bool32>          // (entity, out handle) → found
delegate* unmanaged[Cdecl]<UInt64, AssetHandle, void>             // (entity, handle)

// 反射类型
delegate* unmanaged[Cdecl]<UInt64, ReflectionType, void>          // (entity, typeof(T))
delegate* unmanaged[Cdecl]<UInt64, ReflectionType, Bool32>        // (entity, typeof(T)) → has

// 数组
delegate* unmanaged[Cdecl]<NativeArray<UInt64>>                   // () → entity IDs
delegate* unmanaged[Cdecl]<UInt64, NativeArray<UInt64>>           // (entity) → children
delegate* unmanaged[Cdecl]<AssetHandle, NativeArray<Vector4>, void> // (tex, pixels)
```

---

## 内存管理

| 传递场景 | 分配方 | 释放方 | 方式 |
|----------|--------|--------|------|
| 输出字符串 | C++ (`String::New`) | C# | `using` → `Dispose()` |
| 输出数组 | C++ (`Array<T>::New`) | C# | `using` → `Dispose()` |
| 输出 `ManagedObject` | C++（拷贝含 GCHandle） | C++ | `~ManagedObject()` 时 `GCHandle.Free()` |
| C++ 原生对象 `IntPtr` | C++ (`new`) | C# 显式 | 调用析构 InternalCall |
| Map/Unmap 数组 | C# (`GCHandle.Pinned`) | C# | `Unmap()` → `GCHandle.Free()` |

---

## blittable 结构体布局要求

| C# 侧 | C++ 侧 |
|--------|--------|
| `[StructLayout(LayoutKind.Sequential)]` | POD/Trivial 类型 |
| 字段顺序与 C++ 一致 | 按声明顺序 |
| 必要时加显式 `Padding` 字段 | 对应 `float _padding` |
| **禁止**含 `object`, `string`, `class` 字段 | N/A |
