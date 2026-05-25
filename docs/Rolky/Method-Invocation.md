# Method Invocation

本文说明如何调用 C# 对象的实例方法和静态方法。

## 实例方法调用

### 无返回值

```cpp
Rolky::ManagedObject player = /* ... */;

// 无参调用
player.InvokeMethod("Respawn");

// 带参调用
player.InvokeMethod("TakeDamage", 10.0f);
player.InvokeMethod("SetPosition", 1.0f, 2.0f, 3.0f);
```

### 有返回值

```cpp
// 指定返回类型为模板参数
int health = player.InvokeMethod<int>("GetHealth");
std::string name = player.InvokeMethod<std::string>("GetName");
bool isAlive = player.InvokeMethod<bool>("IsAlive");
```

### 带参数且有返回值

```cpp
// 参数后，返回值通过模板参数指定
float distance = player.InvokeMethod<float>("DistanceTo", otherPlayer);
```

## 静态方法调用

通过 `Type` 对象调用：

```cpp
auto& type = assembly.GetLocalType("MyGame.MathHelper");

// 无返回值
type.InvokeStaticMethod("Log", std::string("Hello from C++!"));

// 有返回值
int result = type.InvokeStaticMethod<int>("Add", 3, 4);
float clamped = type.InvokeStaticMethod<float>("Clamp", value, 0.0f, 1.0f);
```

## 安全调用（Try 版本）

如果方法可能不存在，或者不希望方法抛异常影响流程：

```cpp
// 实例方法 - 无返回值
bool succeeded = player.TryInvokeMethod("OptionalMethod", arg1, arg2);

// 实例方法 - 有返回值
int result;
if (player.TryInvokeMethod("TryCalculate", result, input1, input2)) {
    // 方法调用成功，result 包含返回值
}

// 静态方法 - 有返回值
float value;
if (type.TryInvokeStaticMethod("TryParse", value, std::string("3.14"))) {
    // 解析成功，value 被更新
}
```

Try 版本在方法不存在或调用失败时返回 `false`，不会抛出异常。

## 方法查找规则

Rolky 的方法匹配规则如下：

1. **精确签名匹配**：如果调用时传递的方法名字符串与方法 `ToString()` 输出的完整签名一致，则直接匹配
2. **名称 + 参数类型匹配**：按方法名和参数类型进行匹配
3. **继承搜索**：如果当前类未找到，会向基类搜索

## 支持的参数类型

| C++ 参数类型 | 对应 C# 类型 | 说明 |
|---|---|---|
| `int8_t` | `sbyte` | — |
| `uint8_t` / `std::byte` | `byte` | — |
| `int16_t` | `short` | — |
| `uint16_t` | `ushort` | — |
| `int32_t` | `int` | — |
| `uint32_t` | `uint` | — |
| `int64_t` | `long` | — |
| `uint64_t` | `ulong` | — |
| `float` | `float` | — |
| `double` | `double` | — |
| `bool` | `bool` | 通过 `Bool32` 中转 |
| `std::string` / `Rolky::String` | `string` / `NativeString` | UTF-8 → UTF-16 转换 |
| 任意指针 `T*` | `IntPtr` / `T*` | 指针直接传递 |

## 完整示例

```cpp
auto& calcType = assembly.GetLocalType("MyGame.Calculator");

// 创建实例
auto calc = calcType.CreateInstance();

// 实例方法调用
int sum = calc.InvokeMethod<int>("Add", 10, 20);
std::cout << "10 + 20 = " << sum << std::endl;

// 带字符串参数
std::string result = calc.InvokeMethod<std::string>(
    "FormatResult", "sum", sum);

// 安全调用
float sqrtVal;
if (calc.TryInvokeMethod("TrySqrt", sqrtVal, 16.0f)) {
    std::cout << "sqrt(16) = " << sqrtVal << std::endl;
}

// 静态方法
auto& mathType = assembly.GetLocalType("MyGame.MathUtils");
double pi = mathType.InvokeStaticMethod<double>("GetPI");
std::cout << "PI = " << pi << std::endl;
```

## 注意事项

- 方法名区分大小写，必须与 C# 中的方法名完全一致
- 参数类型匹配是严格的，`int` 和 `float` 不会自动隐式转换
- Try 版本的性能略高于普通版本，因为它不会在 C# 侧产生异常处理开销（查找失败时不抛异常）
- 调用方法时 C# 侧抛出的异常会通过 `ExceptionCallback` 报告
- 方法信息会被缓存（基于 `MethodKey`），重复调用不会重复反射查找
