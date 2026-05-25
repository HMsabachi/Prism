# Getting Started

本文说明如何初始化 Rolky 并启动 .NET 运行时。

## 1. 包含头文件

```cpp
#include <Rolky/HostInstance.hpp>
#include <Rolky/Assembly.hpp>
#include <Rolky/Type.hpp>
#include <Rolky/ManagedObject.hpp>
```

## 2. 创建 HostInstance

`HostInstance` 是 Rolky 的入口点，负责管理 .NET 运行时的生命周期。

```cpp
Rolky::HostInstance host;
```

## 3. 配置 HostSettings

```cpp
Rolky::HostSettings settings;

// 必须：指定 Rolky.Managed.dll 所在目录
settings.RolkyDirectory = "path/to/rolky/bin";

// 可选：设置日志回调
settings.MessageCallback = [](std::string_view message, Rolky::MessageLevel level) {
    std::cout << "[Rolky] " << message << std::endl;
};

// 可选：设置日志过滤级别
settings.MessageFilter = Rolky::MessageLevel::All;

// 可选：设置异常回调
settings.ExceptionCallback = [](std::string_view message) {
    std::cerr << "[Rolky Exception] " << message << std::endl;
};
```

## 4. 初始化

```cpp
auto status = host.Initialize(settings);

if (status != Rolky::RolkyInitStatus::Success) {
    // 处理初始化失败
    switch (status) {
        case Rolky::RolkyInitStatus::DotNetNotFound:
            // 找不到 .NET 运行时
            break;
        case Rolky::RolkyInitStatus::RolkyManagedNotFound:
            // 找不到 Rolky.Managed.dll
            break;
        case Rolky::RolkyInitStatus::RolkyManagedInitError:
            // Rolky.Managed 初始化失败
            break;
    }
}
```

## 5. 完整的初始化示例

```cpp
#include <Rolky/HostInstance.hpp>
#include <iostream>

int main() {
    Rolky::HostInstance host;

    Rolky::HostSettings settings;
    settings.RolkyDirectory = "ThirdParty/Rolky/bin";
    settings.MessageCallback = [](std::string_view msg, Rolky::MessageLevel level) {
        std::cout << "[Rolky] " << msg << std::endl;
    };

    if (host.Initialize(settings) != Rolky::RolkyInitStatus::Success) {
        std::cerr << "Failed to initialize Rolky!" << std::endl;
        return 1;
    }

    // ... 使用 Rolky API ...

    host.Shutdown();
    return 0;
}
```

## 6. 注意事项

- **一个进程通常只需要一个 `HostInstance`**
- `HostInstance::Initialize()` 必须在你使用任何 Rolky API 之前调用
- 初始化时会自动搜索并加载 `hostfxr.dll`（或 `libhostfxr.so`/`.dylib`），然后通过它启动 .NET 运行时
- `RolkyDirectory` 必须包含 `Rolky.Managed.dll` 和 `Rolky.Managed.runtimeconfig.json`
- 初始化完成后可以通过 `host.CreateAssemblyLoadContext()` 创建加载上下文来加载你的程序集
