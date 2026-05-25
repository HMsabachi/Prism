# Marshalling

本文说明 Rolky 的类型映射规则和参数编组行为。理解这些规则有助于正确传递参数和解析返回值。

## 类型映射表

### 基本类型映射

当你在 C++ 端调用 C# 方法或读写字段时，C++ 类型和 C# 类型的映射关系如下：

| C++ 类型 | ManagedType 枚举 | C# 类型 | 说明 |
|---|---|---|---|
| `int8_t` | `SByte` | `sbyte` | 有符号 8 位 |
| `uint8_t` / `std::byte` | `Byte` | `byte` | 无符号 8 位 |
| `int16_t` | `Short` | `short` | 有符号 16 位 |
| `uint16_t` | `UShort` | `ushort` | 无符号 16 位 |
| `int32_t` | `Int` | `int` | 有符号 32 位 |
| `uint32_t` | `UInt` | `uint` | 无符号 32 位 |
| `int64_t` | `Long` | `long` | 有符号 64 位 |
| `uint64_t` | `ULong` | `ulong` | 无符号 64 位 |
| `float` | `Float` | `float` | 32 位浮点 |
| `double` | `Double` | `double` | 64 位浮点 |
| `bool` | `Bool` | `bool` | 通过 `Bool32`(4 字节) 中转 |
| `std::string` / `Rolky::String` | `String` | `string` | UTF-8 ↔ UTF-16 转换 |
| `T*` / `void*` | `Pointer` | `IntPtr` / `T*` | 直接传递指针值 |

## C++ → C# 参数传递

### 方法调用

```cpp
// Rolky 自动编组参数
obj.InvokeMethod("Method", 
    int32_t(42),        → C# int
    float(3.14f),        → C# float
    std::string("Hi"),   → C# string (UTF-8 → UTF-16)
    bool(true)           → C# bool (通过 Bool32)
);
```

编组内部流程：
1. 每个参数被展开为 `(const void*, ManagedType)` 对
2. `const void*` 指向参数值的副本
3. `ManagedType` 告知 C# 端参数的类型编码
4. C# 端 `Marshalling.MarshalParameterArray()` 根据签名将原生指针数组转为 `object?[]`

### 字符串的特殊处理

`std::string`（UTF-8）在传递时：
1. C++ 端通过 `StringHelper::ConvertUtf8ToWide()` 转为 UTF-16（Windows 宽字符）
2. 通过 `Memory::StringToCoTaskMemAuto()` 分配为 `NativeString`
3. C# 端从 `NativeString` 解析为 `string`

### bool 的特殊处理

`bool` 在 C++ 中是 1 字节，而 .NET 中 `bool` 的 Marshal 大小是 4 字节。Rolky 使用 `Bool32`（`uint32_t`）作为中间类型：
- `true` → `Bool32(1)` → C# `bool(true)`
- `false` → `Bool32(0)` → C# `bool(false)`

## C# → C++ 返回值

### 读取返回值

```cpp
// 返回值通过模板参数指定类型
int result = obj.InvokeMethod<int>("GetValue");
float val = obj.InvokeMethod<float>("GetFloat");
std::string name = obj.InvokeMethod<std::string>("GetName");
bool flag = obj.InvokeMethod<bool>("IsActive");
```

### 返回值编组规则

C# 端 `Marshalling.MarshalReturnValue()` 处理以下情况：

| C# 返回类型 | C++ 接收方式 | 说明 |
|---|---|---|
| 值类型 (`int`, `float` 等) | 直接内存拷贝 | Pinned 后 `MemoryCopy` |
| `string` | `std::string` / `Rolky::String` | 通过 `NativeString` 中转 |
| `bool` | `bool` | 通过 `Bool32` 中转 |
| 数组 (SZArray) | `Rolky::Array<T>` | 通过 `ArrayStorage` 固定 |
| 指针 / `IntPtr` | `void*` | 直接指针值拷贝 |
| 枚举 | `int32_t`（底层值） | 按底层类型大小拷贝 |

### 字符串返回值特殊处理

`ManagedObject::GetFieldValue<std::string>()` 和 `InvokeMethod<std::string>()` 的模板特化会自动释放中间 `NativeString`：

```cpp
// 内部流程：
// 1. C# 返回 NativeString（CoTaskMem 分配）
// 2. C++ 读取为 Rolky::String
// 3. 转换为 std::string
// 4. 自动 String::Free()
```

## 参数传递的内部机制

### 参数数组构造

当调用 `InvokeMethod("Func", arg1, arg2, arg3)` 时，Rolky 构造：

```
const void* argumentsArr[] = { &arg1, &arg2, &arg3 };
ManagedType types[] = { GetManagedType<Arg1>(), GetManagedType<Arg2>(), GetManagedType<Arg3>() };
```

这些数组通过函数指针传递给 C# 端的 `InvokeMethod` 函数。

### C# 端反编组

C# 端通过反射获取 `MethodInfo`，然后根据参数类型逐个处理：

1. 读取 `IntPtr` 数组（每个元素指向一个参数）
2. 对每个参数，根据参数声明类型调用 `MarshalPointer(ptr, type)`
3. `MarshalPointer` 的分派逻辑：
   - `IntPtr` / 指针 → 直接返回
   - `bool` → 读单字节，非零为 `true`
   - `string` → 读 `NativeString`，再转 `string`
   - `NativeString` → 读结构体
   - SZArray → 调用 `MarshalArray()`
   - class 类型 → 读 `GCHandle`，取 `Target`
   - 值类型 → `Marshal.PtrToStructure`

## 最佳实践

1. **优先使用模板 API**：如 `InvokeMethod<T>()`、`GetFieldValue<T>()`，它们自动处理编组
2. **避免手动构造 Rolky::String**：直接使用 `std::string`，模板会处理转换
3. **注意性能热点**：大量字符串传递涉及 UTF-8/UTF-16 转换，在性能敏感路径上可考虑缓存或使用其他方案
4. **小心指针生命周期**：传递指针时确保指向的内存在调用期间有效
