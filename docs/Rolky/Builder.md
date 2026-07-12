# Builder — C# 项目编译

## 前提条件

- 安装了 .NET 9 SDK

## 定义解决方案

```cpp
#include <Rolky/ScriptSolution.hpp>

Rolky::ScriptSolution sln;
sln.Name = "Game";
sln.Directory = "Assets/Scripts";
sln.OutputDirectory = "Assets/Scripts/Build";

auto& game = sln.AddProject("Game");
game.SourceFiles = { "Player.cs", "Enemy/" };  // 文件或目录，目录会递归搜 *.cs
game.References = { "Prism.Scripting.dll", "Rolky.Managed.dll" };
game.Defines = { "PR_DEBUG" };

auto& core = sln.AddProject("Prism.Scripting");
core.Directory = "Engine/API";
core.SourceFiles = { "src/" };
core.References = { "Rolky.Managed.dll" };
game.Dependencies = { &core };
```

## 生成解决方案文件

```cpp
sln.Generate();   // 写入 Game.sln + 各项目的 .csproj
```

## 编译

```cpp
#include <Rolky/Builder.hpp>

Rolky::BuildManager builder;
builder.SetLogsDirectory("Assets/Scripts/Logs");

bool ok = builder.Build(sln, "Debug");
bool ok = builder.Build(sln, "Release", true);  // rebuild
```

## 完整示例

```cpp
#include <Rolky/ScriptSolution.hpp>
#include <Rolky/Builder.hpp>

Rolky::ScriptSolution sln;
sln.Name = "Game";
sln.Directory = "Assets/Scripts";
sln.OutputDirectory = "Assets/Scripts/Build";

auto& game = sln.AddProject("Game");
game.SourceFiles = { "Player.cs", "Enemy/" };
game.References = {
    "Assets/Scripts/Prism.Scripting.dll",
    "Assets/Scripts/Rolky.Managed.dll"
};
game.Defines = { "PR_DEBUG" };

Rolky::BuildManager builder;
builder.SetLogsDirectory("Assets/Scripts/Logs");

if (builder.Build(sln, "Debug"))
{
    auto& assembly = alc.LoadAssembly("Assets/Scripts/Build/Game.dll");
    assembly.UploadInternalCalls();
}
```

## 错误诊断

编译失败时，日志在 `SetLogsDirectory` 指定的目录下的 `msbuild_log.txt`。
