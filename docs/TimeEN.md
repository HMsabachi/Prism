
---

# Prism Engine - Time Module Technical Documentation

**Version**: Early Development (March 2026)  
**Primary Function**: Provides time management for the game loop, mirroring `UnityEngine.Time`.  
**Updated**: March 26, 2026

This document records the interfaces, implementation details, and usage of the `Time` class. Please update this document synchronously after any modifications to the Time module.

## 1. Module Overview

The Prism `Time` class utilizes a **static class** design. It is internally driven by the `Application` class via friend access, while external access is restricted to Getters/Setters.  
It uses the high-precision `std::chrono::steady_clock` and supports features such as time scaling and fixed time steps.

**Core Design Principles:**
- All public interfaces are `static`.
- Private static members use the `s_` prefix.
- Internal functions (e.g., `Initialize`, `Update`) are strictly reserved for call-by-`Application`.

## 2. Core Interfaces

**Path**: `Prism/Prism/Core/Time.h`

```cpp
class PRISM_API Time
{
public:
    // Getters
    static float GetDeltaTime() noexcept;
    static float GetUnscaledDeltaTime() noexcept;
    static float GetTime() noexcept;
    static float GetUnscaledTime() noexcept;
    static float GetFixedDeltaTime() noexcept;
    static long long GetFrameCount() noexcept;
    static float GetTimeScale() noexcept;

    // Setters
    static void SetTimeScale(float scale) noexcept;
};
```

## 3. Property Reference Table

| Prism Interface | Description | Affected by TimeScale? |
| :--- | :--- | :---: |
| `GetDeltaTime()` | Time interval between frames. | **Yes** |
| `GetUnscaledDeltaTime()` | Real-time interval between frames. | No |
| `GetTime()` | Total time elapsed since game start. | **Yes** |
| `GetUnscaledTime()` | Real total time (recommended for system timers). | No |
| `GetFixedDeltaTime()` | Fixed update step length. | No |
| `GetTimeScale()` / `SetTimeScale()` | Controls the rate at which time elapses. | - |
| `GetFrameCount()` | Total number of frames rendered. | - |

## 4. Usage Examples

**Common Code Snippets**:

```cpp
// Smooth movement
position += velocity * Time::GetDeltaTime();

// UI Countdown (unaffected by pause/slow-mo)
timer -= Time::GetUnscaledDeltaTime();

// Slow motion effect
Time::SetTimeScale(0.3f);

// Pause the game
Time::SetTimeScale(0.0f);
```

---

