# Managed Objects

本文说明如何创建和操作 C# 托管对象的实例。

## 创建对象实例

使用 `Type::CreateInstance()` 模板方法。参数会被自动编组并传递给 C# 构造函数。

### 无参构造

```cpp
auto& type = assembly.GetLocalType("MyGame.Player");
Rolky::ManagedObject player = type.CreateInstance();
```

### 带参数构造

```cpp
// 创建带参数的实例
auto& type = assembly.GetLocalType("MyGame.Vector3");
auto vec = type.CreateInstance(1.0f, 2.0f, 3.0f);
```

参数可以是任意 Rolky 支持的类型（int、float、bool、std::string、指针等），`CreateInstance` 会在运行时自动匹配最佳构造函数。匹配策略是从当前类向基类遍历查找。

## 对象生命周期

### 复制

```cpp
Rolky::ManagedObject obj1 = type.CreateInstance();
Rolky::ManagedObject obj2 = obj1;  // 复制：创建新的 GCHandle（引用计数+1）
```

### 移动

```cpp
Rolky::ManagedObject obj1 = type.CreateInstance();
Rolky::ManagedObject obj2 = std::move(obj1);  // 移动：转移所有权，obj1 置空
```

### 销毁

```cpp
// 析构时自动释放 GCHandle
{
    Rolky::ManagedObject obj = type.CreateInstance();
    // ... 使用 obj ...
}  // obj 析构，GCHandle 被释放

// 或显式销毁
obj.Destroy();
```

### 判断有效性

```cpp
if (obj.IsValid()) {
    // obj 的 Handle 和 Type 均有效
}
```

## 获取对象类型

```cpp
const Rolky::Type& objType = obj.GetType();
std::cout << objType.GetFullName() << std::endl;
```

`GetType()` 会在首次调用时向 C# 端查询类型并缓存结果。

## 完整示例

```cpp
auto& assembly = alc.LoadAssembly("MyGame.dll");
auto& playerType = assembly.GetLocalType("MyGame.Player");

if (!playerType) {
    std::cerr << "Player type not found!" << std::endl;
    return;
}

// 创建 Player 实例（假设构造函数接受名字字符串）
auto player = playerType.CreateInstance(std::string("Alice"));

if (player.IsValid()) {
    std::cout << "Player created!" << std::endl;

    // 调用方法
    auto health = player.InvokeMethod<int>("GetHealth");
    std::cout << "Health: " << health << std::endl;

    // 读写字段
    player.SetFieldValue("m_Level", 5);
    int level = player.GetFieldValue<int>("m_Level");

    // 对象析构时自动释放
}
```

## 注意事项

- `ManagedObject` 内部持有的是 `GCHandle`，析构时自动释放。不需要手动释放除非使用 `Destroy()`
- 复制操作会创建新的 `GCHandle(Normal)`，指向同一个托管对象
- 移动操作转移所有权，原对象变为无效
- 创建对象时如果找不到匹配的构造函数会返回无效对象（`IsValid() == false`）
