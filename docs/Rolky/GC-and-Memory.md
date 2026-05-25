# GC and Memory Management

本文说明如何控制 .NET 垃圾回收以及使用 Rolky 的内存管理工具。

## 垃圾回收

### 完全回收

```cpp
// 触发一次完全的垃圾回收（等效于 System.GC.Collect()）
Rolky::GC::Collect();
```

### 带参数的回收

```cpp
// 指定代、模式、是否阻塞、是否压缩
Rolky::GC::Collect(
    2,                                      // 回收第 2 代
    Rolky::GCCollectionMode::Aggressive,    // 激进回收模式
    true,                                   // 阻塞等待完成
    true                                    // 压缩堆
);
```

### 等待终结器

```cpp
// 等待所有待处理的终结器执行完成
Rolky::GC::WaitForPendingFinalizers();
```

通常在 `Collect()` 后调用，确保所有回收的对象都已执行终结器。

### GCCollectionMode 说明

| 模式 | 行为 |
|---|---|
| `Default` | 等价于 `Forced` |
| `Forced` | 立即强制回收 |
| `Optimized` | 让 GC 自行判断是否需要回收 |
| `Aggressive` | 尽可能回收更多内存（激进模式） |

### 典型使用场景

```cpp
// 在卸载 ALC 后进行彻底清理
host.UnloadAssemblyLoadContext(alc);

// 回收释放的对象
Rolky::GC::Collect(2, Rolky::GCCollectionMode::Forced, true, true);

// 等待终结器
Rolky::GC::WaitForPendingFinalizers();

// 再次回收以清理终结器复活的对象
Rolky::GC::Collect();
```

## 内存管理

### 跨边界内存分配

Rolky 提供平台无关的内存分配/释放函数，主要用于内部字符串和数组的传递：

```cpp
// AllocHGlobal — 类似 C 的 malloc（Windows 上为 LocalAlloc）
void* ptr = Rolky::Memory::AllocHGlobal(1024);
Rolky::Memory::FreeHGlobal(ptr);

// CoTaskMem 分配 — 用于跨边界字符串传输
auto* str = Rolky::Memory::StringToCoTaskMemAuto(L"Hello");
Rolky::Memory::FreeCoTaskMem(str);
```

### 使用场景

多数情况下你不需要直接调用 `Memory` 中的函数。它们主要用于 Rolky 内部：

- `StringToCoTaskMemAuto` / `FreeCoTaskMem` — `Rolky::String` 的内部内存管理
- `AllocHGlobal` / `FreeHGlobal` — `Rolky::Array` 的内部内存管理

### 对象生命周期管理

`ManagedObject` 的生命周期由内部持有 `GCHandle` 管理：

- **创建**：`CreateInstance()` 在 C# 端调用 `GCHandle.Alloc(obj, Normal)`，返回 `IntPtr`
- **复制**：复制构造时调用 `CopyObject()`，创建新的 `GCHandle(Normal)` 指向同一对象
- **移动**：移动构造直接转移 `GCHandle` 的所有权
- **析构**：`~ManagedObject()` 自动调用 `DestroyObject()` 释放 `GCHandle`

### Debug 模式下的追踪

在 Debug 构建中，Rolky 会追踪所有分配的 `GCHandle`。当卸载 ALC 时，如果有未释放的 `GCHandle`，会在日志中警告：

```
[Rolky](Warn): Found still-registered handle 'xxx' from assembly 'MyAssembly'
[Rolky](Warn): Found unfreed object 'yyy' from assembly 'MyAssembly'. Deallocating.
```

这意味着你有资源泄漏。确保所有 `ManagedObject` 在使用完毕后被正确销毁。

### 最佳实践

```cpp
// ✅ 利用 RAII 自动管理生命周期
{
    auto obj = type.CreateInstance();
    // 使用 obj
} // 自动析构

// ❌ 避免手动管理，容易忘记
auto obj = type.CreateInstance();
// ...
// obj.Destroy();  // 容易忘记
```
