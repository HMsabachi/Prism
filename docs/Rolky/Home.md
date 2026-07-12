# Rolky — C++/C# 互操作引擎

## 概述

Rolky 是一个轻量级的 C++/C# 互操作引擎，设计理念受 Mono 的 P/Invoke 和 InternalCall 机制启发。它允许 C++ 原生代码加载 .NET 程序集、创建托管对象、调用方法、读写字段/属性，并支持 C# 侧通过函数指针反向调用 C++ 代码。

所有交互都基于 .NET 的 `hostfxr` 托管接口和 `[UnmanagedCallersOnly]` 机制，没有额外的 IPC 或序列化开销。

## 快速导航

| 章节 | 说明 |
|---|---|
| [Getting-Started](Getting-Started) | 初始化 Rolky、配置 HostInstance |
| [Assembly-Loading](Assembly-Loading) | 程序集加载与 ALC 管理 |
| [Type-Reflection](Type-Reflection) | 类型查询与反射 |
| [Managed-Objects](Managed-Objects) | 创建和操作托管对象 |
| [Method-Invocation](Method-Invocation) | 调用实例方法和静态方法 |
| [Field-and-Property-Access](Field-and-Property-Access) | 字段与属性的读写 |
| [InternalCalls](InternalCalls) | C# 调用 C++ 的回调机制 |
| [GC-and-Memory](GC-and-Memory) | 垃圾回收和内存管理 |
| [Interop-Types](Interop-Types) | 字符串、数组等跨边界类型 |
| [Marshalling](Marshalling) | 类型映射与参数编组 |
| [InternalCall-Patterns](InternalCall-Patterns) | 传参模式：对象/数组/字符串/结构体 |
| [Builder](Builder) | C# 项目编译与诊断 |

## 架构示意

```
┌───────────────────┐      ┌───────────────────┐
│   C++ 应用代码     │◄────►│   C# 托管代码     │
│  (Rolky API)      │      │  (你的程序集)     │
├───────────────────┤      ├───────────────────┤
│  HostInstance     │      │  ManagedObject    │
│  AssemblyLoadCtx  │      │  TypeInterface    │
│  Type / MethodInfo│      │  InternalCalls    │
│  ManagedObject    │      │  AssemblyLoader   │
├───────────────────┤      ├───────────────────┤
│  Rolky.Native     │◄────►│  Rolky.Managed    │
│  (C++ 桥接层)     │      │  (C# 桥接层)      │
└────────┬──────────┘      └────────┬──────────┘
         │                         │
         └──────────┬──────────────┘
                    │
         ┌──────────▼──────────┐
         │  .NET Runtime (CLR) │
         │  hostfxr / coreclr  │
         └─────────────────────┘
```

## 前提条件

- .NET 9 Runtime（hostfxr 搜索路径见下文）
- C++17 或更新的编译器
- Windows/Linux/macOS 均支持

## hostfxr 搜索路径

Rolky 自动在以下路径搜索 .NET 运行时：

- **Windows**: `%ProgramFiles%/dotnet/host/fxr/<version>/`
- **macOS**: `/usr/local/share/dotnet/host/fxr/` 或 `/usr/share/dotnet/host/fxr/`
- **Linux**: `/usr/local/lib/dotnet/host/fxr/`、`/usr/lib/dotnet/host/fxr/` 等

当前优先级选择以 `'9'` 开头版本号的目录（.NET 9）。
