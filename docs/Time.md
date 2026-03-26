
# Prism Engine - 时间模块技术文档（Time）

**版本**：早期开发阶段（2026年3月）  
**主要功能**：提供游戏循环所需的时间管理，模仿 UnityEngine.Time  
**更新日期**：2026年3月26日

本文档记录 Time 类的接口、实现细节和使用方式。每次修改 Time 模块后请同步更新此文档。

## 1. 时间模块总览

Prism 的 `Time` 类采用**静态类**设计，通过 `Application` 作为友元进行内部调用，外部仅通过 Getter/Setter 访问。  
使用高精度 `std::chrono::steady_clock`，支持时间缩放、固定时间步等功能。

核心设计：
- 所有公开接口均为 `static`
- 私有静态成员使用 `s_` 前缀
- `Initialize/Update` 等内部函数仅允许 `Application` 调用

## 2. 核心接口

位置：`Prism/Prism/Core/Time.h`

```cpp
class PRISM_API Time
{
public:
    // Getter
    static float GetDeltaTime() noexcept;
    static float GetUnscaledDeltaTime() noexcept;
    static float GetTime() noexcept;
    static float GetUnscaledTime() noexcept;
    static float GetFixedDeltaTime() noexcept;
    static long long GetFrameCount() noexcept;
    static float GetTimeScale() noexcept;

    // Setter
    static void SetTimeScale(float scale) noexcept;
};
```

## 3. 属性对应表

| Prism 接口                    | 说明                                      | 是否受 TimeScale 影响 |
|-------------------------------|-------------------------------------------|-----------------------|
| `GetDeltaTime()`              | 每帧时间间隔                              | 是                    |
| `GetUnscaledDeltaTime()`      | 真实每帧时间间隔                        | 否                    |
| `GetTime()`                   | 游戏启动至今总时间                        | 是                    |
| `GetUnscaledTime()`           | 真实总时间（推荐用于计时器）              | 否                    |
| `GetFixedDeltaTime()`         | 固定更新步长                              | 否                    |
| `GetTimeScale()` / `SetTimeScale()` | 时间缩放                                  | -                     |
| `GetFrameCount()`             | 已渲染帧数                                | -                     |

## 4. 使用示例（在 Application 主循环中）


**常用代码片段**：
```cpp
// 平滑移动
position += velocity * Time::GetDeltaTime();

// UI 倒计时（不受暂停影响）
timer -= Time::GetUnscaledDeltaTime();

// 慢动作
Time::SetTimeScale(0.3f);

// 暂停
Time::SetTimeScale(0.0f);
```

