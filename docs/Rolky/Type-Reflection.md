# Type Reflection

本文说明如何使用 Rolky 查询 .NET 类型信息。

## 获取类型

### 通过程序集获取类型

```cpp
auto& assembly = alc.LoadAssembly("MyAssembly.dll");

// 获取所有本地类型（当前 ALC 内）
const auto& localTypes = assembly.GetLocalTypes();
for (const auto& type : localTypes) {
    std::cout << type.GetFullName() << std::endl;
}

// 按名称查找类型
auto& myType = assembly.GetLocalType("MyNamespace.MyClass");
if (myType) {  // operator bool 检查类型是否有效
    // 使用类型...
}

// 按 ID 查找类型
auto& typeById = assembly.GetLocalType(typeId);
```

### 判断类型有效性

```cpp
if (myType) {
    // 类型有效（TypeId != -1）
}
```

## 类型基本信息

```cpp
// 完整名称（如 "System.Collections.Generic.List`1[[System.String]]"）
Rolky::String fullName = type.GetFullName();

// 程序集限定名称（包含程序集信息，适用于 Type.GetType()）
Rolky::String qualifiedName = type.GetAssemblyQualifiedName();

// 类型大小（Marshal.SizeOf 结果）
int32_t size = type.GetSize();

// 托管类型枚举
Rolky::ManagedType managedType = type.GetManagedType();
```

## 类型关系判断

```cpp
// 判断是否为子类
if (myType.IsSubclassOf(baseType)) {
    // myType 继承自 baseType
}

// 判断是否可赋值给另一个类型（isAssignableTo）
if (myType.IsAssignableTo(interfaceType)) {
    // myType 实现了 interfaceType
}

// 反向判断
if (myType.IsAssignableFrom(derivedType)) {
    // myType 可以被 derivedType 赋值
}
```

## 基类和接口

```cpp
// 获取基类
Rolky::Type& baseType = type.GetBaseType();

// 获取实现的接口列表
std::vector<Rolky::Type*>& interfaces = type.GetInterfaceTypes();
for (auto* interfaceType : interfaces) {
    std::cout << "Implements: " << interfaceType->GetFullName() << std::endl;
}
```

## 数组类型

```cpp
// 判断是否为 SZArray（单维零下限数组）
if (type.IsSZArray()) {
    // 获取元素类型
    Rolky::Type& elementType = type.GetElementType();
}
```

## 方法查询

```cpp
// 获取所有方法
auto methods = type.GetMethods();

// 检查是否存在特定方法
if (type.HasMethod("MyMethod")) {
    // 存在该方法
}
```

## 字段查询

```cpp
// 获取所有字段
auto fields = type.GetFields();
for (const auto& field : fields) {
    std::cout << "Field: " << field.GetName() << std::endl;
}
```

## 属性查询

```cpp
// 获取所有属性
auto properties = type.GetProperties();
for (const auto& prop : properties) {
    std::cout << "Property: " << prop.GetName() << std::endl;
}
```

## 特性（Attribute）查询

```cpp
// 检查是否有特定特性
Rolky::Type& myAttrType = ...;
if (type.HasAttribute(myAttrType)) {
    // 类型标记了指定特性
}

// 获取所有特性
auto attributes = type.GetAttributes();
for (auto& attr : attributes) {
    // attr.GetType() 获取特性类型
    // attr.GetFieldValue<T>("fieldName") 读取特性中的字段值
}
```

## 类型比较

```cpp
// 类型按 TypeId 比较
if (type1 == type2) {
    // 同一类型
}
```

## 完整示例

```cpp
auto& assembly = alc.LoadAssembly("MyGame.dll");
auto& playerType = assembly.GetLocalType("MyGame.Player");

if (!playerType) {
    std::cerr << "Type not found!" << std::endl;
    return;
}

std::cout << "Type: " << playerType.GetFullName() << std::endl;
std::cout << "Base class: " << playerType.GetBaseType().GetFullName() << std::endl;

// 枚举方法
auto methods = playerType.GetMethods();
for (const auto& method : methods) {
    std::cout << "  Method: " << method.GetName()
              << " (access: " << static_cast<int>(method.GetAccessibility()) << ")"
              << std::endl;
}

// 枚举字段
auto fields = playerType.GetFields();
for (const auto& field : fields) {
    std::cout << "  Field: " << field.GetName()
              << " type: " << field.GetType().GetFullName()
              << std::endl;
}
```

## 注意事项

- `GetLocalType()` 只查找当前 ALC 中的类型。`GetType()`（已弃用）会搜索全局缓存
- 类型名称必须使用完整的命名空间限定名（如 `"System.String"` 而非 `"String"`）
- `Type::operator bool()` 可用于快速检查类型是否存在
