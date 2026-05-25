# Assembly Loading

本文说明如何加载 C# 程序集以及管理 AssemblyLoadContext（ALC）。

## AssemblyLoadContext 的概念

.NET 的 `AssemblyLoadContext`（ALC）提供了一种隔离机制，允许在同一个进程中加载和卸载多个版本的程序集。Rolky 将其暴露给 C++ 端。

## 创建 AssemblyLoadContext

```cpp
// 创建默认的 ALC
auto alc = host.CreateAssemblyLoadContext("MyApp");

// 创建带额外 DLL 搜索路径的 ALC
// InDllPath 是一个冒号分隔的路径列表，AssemblyLoader 会在这些路径中解析依赖
auto alcWithPaths = host.CreateAssemblyLoadContext(
    "MyApp",
    "C:/MyApp/Plugins:D:/SharedLibs"
);
```

**注意**：`InDllPath` 只影响 C# 侧 `AssemblyLoadContext` 运行时解析依赖的行为（即 `Resolving` 事件中的搜索），不影响 `LoadAssembly` 的直接路径加载。

## 加载程序集

### 从文件加载

```cpp
auto& assembly = alc.LoadAssembly("C:/MyApp/MyAssembly.dll");

if (assembly.GetLoadStatus() == Rolky::AssemblyLoadStatus::Success) {
    std::cout << "Loaded: " << assembly.GetName() << std::endl;
} else {
    // 处理加载失败
}
```

### 从内存加载

```cpp
// 读取文件到缓冲区
std::ifstream file("assembly.dll", std::ios::binary | std::ios::ate);
auto size = file.tellg();
std::vector<std::byte> buffer(size);
file.seekg(0);
file.read(reinterpret_cast<char*>(buffer.data()), size);

// 从内存加载
auto& assembly = alc.LoadAssemblyFromMemory(buffer.data(), buffer.size());
```

## 获取已加载的程序集

```cpp
const auto& assemblies = alc.GetLoadedAssemblies();
for (size_t i = 0; i < assemblies.GetElementCount(); ++i) {
    const auto& asm = assemblies[i];
    std::cout << "Assembly: " << asm.GetName() << std::endl;
}
```

## 卸载 AssemblyLoadContext

```cpp
host.UnloadAssemblyLoadContext(alc);
```

卸载时 Rolky 会自动：
1. 清空类型缓存（`TypeInterface.s_CachedTypes` 等）
2. 清除方法反射缓存
3. 收集并报告未释放的 GCHandle（仅 Debug 模式）
4. 调用 ALC 的 `Unload()` 方法

## 加载状态处理

```cpp
auto& assembly = alc.LoadAssembly("missing.dll");

switch (assembly.GetLoadStatus()) {
    case Rolky::AssemblyLoadStatus::Success:
        // 加载成功
        break;
    case Rolky::AssemblyLoadStatus::FileNotFound:
        // 文件不存在
        break;
    case Rolky::AssemblyLoadStatus::FileLoadFailure:
        // 文件加载失败（如权限问题）
        break;
    case Rolky::AssemblyLoadStatus::InvalidFilePath:
        // 路径为空或格式错误
        break;
    case Rolky::AssemblyLoadStatus::InvalidAssembly:
        // 不是有效的 .NET 程序集（BadImageFormat）
        break;
    case Rolky::AssemblyLoadStatus::UnknownError:
        // 其他未知错误
        break;
}
```

## 最佳实践

1. **为不同用途创建不同的 ALC**：例如插件系统应为每个插件创建独立的 ALC，以便独立卸载
2. **依赖管理**：如果程序集有依赖项，在创建 ALC 时指定 `InDllPath` 或者在 `RolkyDirectory` 同目录放置依赖
3. **卸载前确保释放**：在卸载 ALC 之前，确保所有该 ALC 中的 `ManagedObject` 已被销毁
