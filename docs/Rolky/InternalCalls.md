# Internal Calls

InternalCall 是 Rolky 的核心机制，它允许 C# 代码直接调用 C++ 函数，而无需经过 P/Invoke 或任何中间层。这是通过将 C++ 函数指针注册到 C# 端的静态函数指针字段上实现的。

## 概念

```
C++ 端定义函数 → 注册到 C# 端 → C# 代码直接调用
```

注册后，C# 端的调用会被 JIT 直接编译为对原生函数指针的调用，几乎没有额外开销。

## 使用步骤

### 步骤 1：C# 端声明函数指针字段

在 C# 中创建一个类，包含 `static` 的**函数指针字段**：

```csharp
// MyGame.InternalCalls.cs
namespace MyGame;

internal static class InternalCalls
{
    // 声明一个函数指针字段
    // 使用 delegate* 语法（C# 9+）
    internal static unsafe delegate*<string, void> s_Log_Trace;
    
    // 带返回值的函数指针
    internal static unsafe delegate*<int, int, int> s_Math_Add;
    
    // 参数可以是任意托管类型
    internal static unsafe delegate*<string, int, void> s_Print_Message;
}
```

> **重要**：字段必须是 `static`、`internal`（或更低访问级别）、类型为 `delegate*<...>`

### 步骤 2：C++ 端注册函数指针

```cpp
// 假设你已经加载了一个程序集
auto& assembly = alc.LoadAssembly("MyGame.dll");

// 注册函数到 C# 端的 InternalCalls 类
assembly.AddInternalCall(
    "MyGame.InternalCalls",   // 类型完整名称（包含命名空间）
    "s_Log_Trace",            // 静态函数指针字段名
    (void*)LogTrace           // C++ 函数指针
);

// 批量注册多个 InternalCall
assembly.AddInternalCall("MyGame.InternalCalls", "s_Math_Add",    (void*)Add);
assembly.AddInternalCall("MyGame.InternalCalls", "s_Print_Message", (void*)PrintMessage);

// 上传所有 InternalCall 到 C# 端
assembly.UploadInternalCalls();
```

### 步骤 3：C++ 端实现函数

```cpp
// C++ 实现
void LogTrace(const char* message) {
    std::cout << "[Trace] " << message << std::endl;
}

int Add(int a, int b) {
    return a + b;
}

void PrintMessage(const char* message, int repeat) {
    for (int i = 0; i < repeat; i++) {
        std::cout << message << std::endl;
    }
}
```

> **重要**：C++ 函数签名必须与 C# 端的 `delegate*` 声明完全匹配。

### 步骤 4：C# 端调用

注册完成后，C# 代码可以直接调用字段（即调用 C++ 函数）：

```csharp
// 这个调用会直接跳转到 C++ 的 LogTrace 函数
InternalCalls.s_Log_Trace("Hello from C#!");

int result = InternalCalls.s_Math_Add(3, 4);

InternalCalls.s_Print_Message("Hello!", 3);
```

## 名称解析规则

`AddInternalCall` 的第一个参数格式为：

```
"Namespace.TypeName+FieldName, AssemblyName"
```

Rolky 自动拼接此格式，你只需要分别传入：
- `InClassName`：如 `"MyGame.InternalCalls"`
- `InVariableName`：如 `"s_Log_Trace"`

它会自动组装为 `"MyGame.InternalCalls+s_Log_Trace, AssemblyName"`，其中 `AssemblyName` 来自程序集的元数据。

## 完整的示例

### C++ 端

```cpp
#include <Rolky/HostInstance.hpp>
#include <Rolky/Assembly.hpp>
#include <iostream>

// C++ 函数实现
static void LogInfo(const char* msg) {
    std::cout << "[Info] " << msg << std::endl;
}

static int AddNumbers(int a, int b) {
    return a + b;
}

int main() {
    Rolky::HostInstance host;
    host.Initialize({ .RolkyDirectory = "Rolky/bin" });

    auto alc = host.CreateAssemblyLoadContext("MyApp");
    auto& assembly = alc.LoadAssembly("MyGame.dll");

    // 注册 InternalCall
    assembly.AddInternalCall("MyGame.InternalCalls", "s_LogInfo", (void*)LogInfo);
    assembly.AddInternalCall("MyGame.InternalCalls", "s_AddNumbers", (void*)AddNumbers);
    assembly.UploadInternalCalls();

    // 现在 C# 代码可以调用上述两个 C++ 函数

    host.Shutdown();
    return 0;
}
```

### C# 端

```csharp
// MyGame/InternalCalls.cs
namespace MyGame;

internal static class InternalCalls
{
    internal static unsafe delegate*<string, void> s_LogInfo;
    internal static unsafe delegate*<int, int, int> s_AddNumbers;
}

// MyGame/GameLogic.cs
namespace MyGame;

public class GameLogic
{
    public static void Test()
    {
        InternalCalls.s_LogInfo("This prints from C++!");
        int sum = InternalCalls.s_AddNumbers(10, 20);
    }
}
```

## 最佳实践

1. **按功能模块分组**：建议按功能命名 `InternalCalls` 中的字段，如 `s_Log_Trace`、`s_Log_Info`、`s_Renderer_DrawMesh`
2. **注册时机**：在程序集加载后、调用任何 C# 代码之前完成注册
3. **线程安全**：`UploadInternalCalls()` 是一次性批量操作，注册完成后函数指针字段是只读的，后续调用无需同步
4. **签名一致性**：C++ 函数和 C# `delegate*` 的签名必须完全一致，否则会导致未定义行为
