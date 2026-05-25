# Interop Types

本文说明 Rolky 中用于跨边界数据传输的特殊类型。

## String（跨边界字符串）

`Rolky::String` 是 C++ 端与 C# `NativeString` 对应的类型。两个类型具有相同的内存布局，可以直接跨越边界传递。

### 创建字符串

```cpp
// 从 UTF-8 字符串创建
Rolky::String str1 = Rolky::String::New("Hello World");
Rolky::String str2 = Rolky::String::New(std::string_view("Hello"));

// 转换为 std::string
std::string utf8Str = (std::string)str1;

// 比较
if (str1 == str2) { /* ... */ }
if (str1 == std::string_view("Hello")) { /* ... */ }
```

### 释放字符串

```cpp
// String::New 分配的内存必须由 String::Free 释放
Rolky::String str = Rolky::String::New("temporary");
// 使用 str ...
Rolky::String::Free(str);
```

### ScopedString（RAII 包装）

`ScopedString` 在构造时接管字符串，析构时自动释放：

```cpp
{
    Rolky::ScopedString str = Rolky::String::New("auto-managed");
    std::string s = str;  // 可转换为 std::string
    // 自动释放
}

// 赋值时自动释放旧字符串
Rolky::ScopedString str = Rolky::String::New("first");
str = Rolky::String::New("second");  // "first" 自动释放
```

## Array（跨边界数组）

`Rolky::Array<T>` 是 C++ 端与 C# `NativeArray<T>` 对应的类型。

### 创建数组

```cpp
// 创建指定长度的数组（内存为 AllocHGlobal 分配）
auto arr = Rolky::Array<int>::New(10);
arr[0] = 42;
arr[1] = 100;

// 从 vector 创建
std::vector<float> values = { 1.0f, 2.0f, 3.0f };
auto floatArr = Rolky::Array<float>::New(values);

// 从初始化列表创建
auto strArr = Rolky::Array<const char*>::New({ "a", "b", "c" });
```

### 使用数组

```cpp
// 访问元素
int first = arr[0];
arr[1] = 50;

// 获取属性
size_t len = arr.Length();          // 元素个数
size_t bytes = arr.ByteLength();    // 总字节数
bool empty = arr.IsEmpty();         // 是否为空

// 迭代
for (auto& elem : arr) {
    std::cout << elem << std::endl;
}

// 获取数据指针
int* data = arr.Data();
```

### 释放数组

```cpp
Rolky::Array<int>::Free(arr);
```

## 使用场景

`Rolky::String` 和 `Rolky::Array` 主要在以下场景中使用：

1. **方法调用参数/返回值**：当调用 C# 方法涉及字符串或数组时自动使用
2. **InternalCall 参数**：InternalCall 注册时使用的名称是 `Rolky::String`
3. **反射查询返回值**：`Type::GetFullName()` 等函数返回 `Rolky::String`

大多数情况下，你不需要直接操作这些类型——模板方法会为你自动处理转换：

```cpp
// 无需手动创建 String
obj.InvokeMethod("SetName", std::string("Alice"));

// 无需手动处理返回的 String
std::string name = obj.GetFieldValue<std::string>("m_Name");
```

但当你需要调用原始 API 或处理特殊场景时，了解这些类型是很有帮助的。

## 内存布局参考

| C++ 类型 | 大小 | C# 对应类型 |
|---|---|---|
| `Rolky::String` | 16 字节 | `NativeString` |
| `Rolky::Array<T>` | 32 字节 | `NativeArray<T>` |
| `Rolky::Bool32` | 4 字节 | `Bool32` |
| `Rolky::ReflectionType` | 4 字节 | `ReflectionType` |

这些类型在 C++ 和 C# 两侧具有完全一致的内存布局和成员偏移，确保数据可以直接跨越边界传递而无需序列化。
