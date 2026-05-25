# Field and Property Access

本文说明如何读写 C# 对象的字段（Field）和属性（Property）。

## 字段读写

### 读字段

```cpp
Rolky::ManagedObject player = /* ... */;

// 基本类型字段
int level = player.GetFieldValue<int>("m_Level");
float speed = player.GetFieldValue<float>("m_Speed");
bool isActive = player.GetFieldValue<bool>("m_IsActive");

// 字符串字段
std::string name = player.GetFieldValue<std::string>("m_Name");
```

### 写字段

```cpp
player.SetFieldValue("m_Level", 10);
player.SetFieldValue("m_Speed", 5.5f);
player.SetFieldValue("m_IsActive", true);
player.SetFieldValue("m_Name", std::string("Alice"));
```

### 原始指针方式

如果需要更灵活的类型处理，可以使用 `Raw` 版本：

```cpp
// 读取到已分配的内存
int value;
player.GetFieldValueRaw("m_Level", &value);

// 写入值
float newSpeed = 10.0f;
player.SetFieldValueRaw("m_Speed", &newSpeed);
```

## 属性读写

属性和字段的使用方式完全相同：

```cpp
// 读属性
int health = player.GetPropertyValue<int>("Health");
std::string tag = player.GetPropertyValue<std::string>("Tag");

// 写属性
player.SetPropertyValue("Health", 100);
player.SetPropertyValue("Tag", std::string("Player"));
```

### 属性访问控制

C# 侧会自动检查属性是否有 getter/setter：

- 读属性时如果没有 getter，会记录错误日志
- 写属性时如果没有 setter，会记录错误日志

## 特殊类型处理

### bool 类型

`bool` 类型在跨边界传递时使用 4 字节的 `Bool32`，模板特化版本自动处理转换：

```cpp
// 这些是等价的
bool active = player.GetFieldValue<bool>("m_Active");

// 手动方式（不推荐）
Rolky::Bool32 raw;
player.GetFieldValueRaw("m_Active", &raw);
bool active = raw != 0;
```

### std::string 类型

`std::string` 在跨边界时转换为 `NativeString`（UTF-8 → UTF-16），模板特化自动处理：

```cpp
// 读
std::string name = player.GetFieldValue<std::string>("m_Name");

// 写
player.SetFieldValue("m_Name", std::string("Bob"));
```

## 使用反射信息访问

你也可以通过反射获取字段/属性信息：

```cpp
auto fields = type.GetFields();
for (const auto& field : fields) {
    std::cout << "Field: " << field.GetName()
              << " type: " << field.GetType().GetFullName()
              << " accessibility: " << static_cast<int>(field.GetAccessibility())
              << std::endl;
}

auto properties = type.GetProperties();
for (const auto& prop : properties) {
    std::cout << "Property: " << prop.GetName()
              << " type: " << prop.GetType().GetFullName()
              << std::endl;
}
```

## 完整示例

```cpp
// 创建对象
auto& playerType = assembly.GetLocalType("MyGame.Player");
auto player = playerType.CreateInstance("Alice");

// 读取字段
std::string name = player.GetFieldValue<std::string>("m_Name");
int hp = player.GetFieldValue<int>("m_MaxHP");
float speed = player.GetFieldValue<float>("m_MoveSpeed");

// 修改字段
player.SetFieldValue("m_MaxHP", 200);
player.SetFieldValue("m_MoveSpeed", 8.0f);

// 通过属性访问
int currentHP = player.GetPropertyValue<int>("HP");
player.SetPropertyValue("HP", currentHP + 50);

// 打印结果
std::cout << "Player: " << name
          << " HP: " << player.GetPropertyValue<int>("HP")
          << " Speed: " << player.GetFieldValue<float>("m_MoveSpeed")
          << std::endl;
```

## 注意事项

- 字段/属性**区分大小写**，必须与 C# 代码中的名称完全一致
- 访问不存在的字段/属性会记录错误日志，且不会修改任何值
- 字段查找包含 `Public` 和 `NonPublic`（即私有字段也可访问）的 `Instance` 绑定
- 属性查找同样包含公共和非公共绑定
- 字符串字段的读写会涉及 UTF-8 与 UTF-16 之间的转换，有一定开销
