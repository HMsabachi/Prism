# PythonScriptCore — Python 脚本引擎核心封装层

## 设计目标

对 CPython C API 进行深度封装。所有 Python 交互通过 `ScriptModule`、`ScriptClass`、`ScriptObject` 完成。

## 架构

- **`ScriptValue*`** — 不透明句柄，本质是 `PyObject*`，仅在前向声明，引擎代码不可见
- **`ScriptRef`** — RAII 智能指针，自动管理引用计数
- **`ScriptHost`** — 解释器生命周期
- **`ScriptModule`** — Python 模块
- **`ScriptClass`** — Python 类
- **`ScriptObject`** — Python 实例
- **`NativeModule`** — 向 Python 注册 C++ 函数

## 文件结构

```
Prism/src/Scripting/Python/
├── PythonScriptCore.h      # 封装层头文件
└── PythonScriptCore.cpp    # 实现文件
```

## PythonScriptCore.h API 参考

### ScriptRef — RAII 引用

```cpp
ScriptRef();                        // 空引用
explicit ScriptRef(ScriptValue* v); // 包装 borrowed ref，INCREF
~ScriptRef();                       // DECREF

ScriptRef(const ScriptRef&);        // 拷贝 — INCREF
ScriptRef(ScriptRef&&) noexcept;    // 移动 — 不增减引用

static ScriptRef Adopt(ScriptValue* v); // 接管所有权，不 INCREF
ScriptValue* Detach();                  // 释放所有权，不再 DECREF

ScriptValue* Get() const;           // 获取原始指针

bool IsValid() const;
bool IsNone() const;

ScriptRef GetAttribute(const char* name) const;
void SetAttribute(const char* name, const ScriptRef& value) const;
bool HasAttribute(const char* name) const;
```

### 两种构造方式的区别

```cpp
// 方式一：包装 borrowed ref（自动 INCREF）
Python::ScriptRef ref(existingPyObj);  // ref count +1

// 方式二：接管新对象（不 INCREF）
Python::ScriptRef ref = Python::ScriptRef::Adopt(newPyObj);  // ref count 不变
```

桥接函数中包装 `args` 参数时用方式一，接收 C API 返回的新对象时用方式二。

### ScriptHost — 解释器生命周期

```cpp
Python::ScriptHost::Initialize();   // 启动解释器 + 添加 Assets/scripts/ 到 sys.path
Python::ScriptHost::Shutdown();     // 关闭解释器
Python::ScriptHost::IsInitialized();
```

### ScriptModule — 模块

```cpp
auto mod = Python::ScriptModule::Import("my_script");
if (mod.IsValid()) {
    ScriptRef cls = mod.GetAttribute("MyClass");
}
```

### ScriptClass — 类

```cpp
auto cls = Python::ScriptClass::From(mod, "MyClass");

if (cls.HasMethod("OnCreate")) { ... }

// 读取 Python 类型注解（用于暴露编辑器可编辑字段）
Python::ScriptClass::AnnotationMap annotations = cls.GetAnnotations();
// annotations["speed"] → <class 'float'>

auto obj = cls.CreateInstance();
```

### ScriptObject — 实例

```cpp
Python::ScriptObject obj = cls.CreateInstance();

// 方法调用（Rolky 风格，TReturn 默认 void）
obj.Invoke("OnCreate");                      // 无参无返回值
obj.Invoke("OnUpdate", 3.14f);               // 单参无返回值
obj.Invoke("Move", 1.0f, 2.0f, 3.0f);       // 多参无返回值
// → Python: obj.Move(1.0, 2.0, 3.0)

// 带返回值自动转换
float val = obj.Invoke<float>("GetSpeed");   // Python → float
int32_t cnt = obj.Invoke<int32_t>("GetCount"); // Python → int32_t
std::string s = obj.Invoke<std::string>("GetName"); // Python → string

// 类型化字段读写
float speed = obj.GetField<float>("speed");
obj.SetField("speed", 100.0f);

// 原始内存读写（给 PublicField 子系统用）
void GetFieldRaw(const char* name, void* buffer) const;
void SetFieldRaw(const char* name, const void* buffer) const;
```

### 类型转换函数

```cpp
// C++ → ScriptRef
ScriptRef FloatToValue(float v);
ScriptRef IntToValue(int32_t v);
ScriptRef UInt64ToValue(uint64_t v);
ScriptRef StringToValue(std::string_view v);
ScriptRef BoolToValue(bool v);
ScriptRef NoneValue();

// ScriptRef → C++
float       ValueToFloat(const ScriptRef& v);
int32_t     ValueToInt(const ScriptRef& v);
uint64_t    ValueToUInt64(const ScriptRef& v);
std::string ValueToString(const ScriptRef& v);
bool        ValueToBool(const ScriptRef& v);
```

### 模板派分

```cpp
template<typename T>
ScriptRef ToValue(T&& value);

ScriptRef val = ToValue(42);          // → IntToValue
ScriptRef val = ToValue(3.14f);       // → FloatToValue
ScriptRef val = ToValue("hello");     // → StringToValue
```

### Tuple 操作

```cpp
ScriptRef elements[] = { FloatToValue(1.0f), FloatToValue(2.0f) };
ScriptRef tuple = MakeTuple(elements, 2);

uint32_t sz = GetTupleSize(tuple);
ScriptRef elem = GetTupleElement(tuple, 0);
```

### NativeModule — 向 Python 注册 C++ 函数

```cpp
Python::ScriptValue* MyFunc(Python::ScriptValue* args)
{
    Python::ScriptRef argsRef(args);
    int32_t x = Python::ValueToInt(Python::GetTupleElement(argsRef, 0));
    return Python::NoneValue().Detach();
}

Python::NativeModule mymod("mymodule");
mymod.AddFunction("do_something", MyFunc, "文档说明");
mymod.Register();

// Python: import mymodule; mymodule.do_something(42)
```

#### 桥接函数规则

- 参数 `args` 是 tuple 的 borrowed ref，**必须用 `ScriptRef(argsRef)` 包装**
- 返回值用 `.Detach()` 释放所有权交给 Python
- 签名必须是 `ScriptValue* (*)(ScriptValue*)`

## 实现要点

### GIL 管理

每个调用 CPython API 的包装方法内部独立获取/释放 GIL。

### 引用计数约定

| 场景 | 方式 | 引用计数变化 |
|------|------|-------------|
| 包装 borrowed ref | `ScriptRef(ptr)` | +1 |
| 接管新对象 | `ScriptRef::Adopt(ptr)` | 不变 |
| 返回给 Python | `.Detach()` | 不变（放弃管理权） |
| 拷贝 | 拷贝构造/赋值 | +1 |
| 移动 | 移动构造/赋值 | 不变 |
| 销毁 | 析构 | -1 |

### Invoke 返回值模板

`Invoke` 的第一个模板参数 `TReturn` 默认为 `void`，不取返回值。需要返回值时显式指定：

| 调用方式 | 返回值类型 | 用途 |
|---------|-----------|------|
| `obj.Invoke("method")` | `void` | 丢弃返回值（生命周期回调） |
| `obj.Invoke("method", arg)` | `void` | 同上，带参数 |
| `obj.Invoke<float>("method")` | `float` | 取 Python float → C++ float |
| `obj.Invoke<int32_t>("method")` | `int32_t` | 取 Python int → C++ int32 |
| `obj.Invoke<std::string>("method")` | `std::string` | 取 Python str → C++ string |
| `obj.Invoke<bool>("method")` | `bool` | 取 Python bool → C++ bool |

### 命名约定

所有类名使用 Rolky 风格而非 CPython 风格：
- `ScriptModule`（而非 `PyModule`）
- `ScriptClass`（而非 `PyClass`）
- `ScriptObject`（而非 `PyObject`）
- `ScriptRef`（而非 `PyRef`）
