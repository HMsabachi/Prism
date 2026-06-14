# AGENTS.md

This file provides guidance to Codex (Codex.ai/code) when working with code in this repository.

## Commands

### Build (Windows)
```bash
# Generate Visual Studio 2022 solution files
cmd.exe /c Win-GenerateProjects.bat

# Build from command line (Debug)
msbuild Prism.sln /p:Configuration=Debug /p:Platform=x64

# Build C# scripting project
msbuild PrismManaged.sln /p:Configuration=Debug

# Build a specific project
msbuild Prism.sln /p:Configuration=Debug /p:Platform=x64 /t:Prism
msbuild Prism.sln /p:Configuration=Debug /p:Platform=x64 /t:PrismEditor
```

### Configurations
- **Debug** — Development build, optimizations off, debug symbols on
- **Release** — Optimized build, debug symbols off
- **Dist** — Distribution build (final shipping)

### Output Structure
- C++ binaries go to `bin/<config>-windows-x86_64/<project>/`
- C# assemblies go to `bin/<config>-windows-x86_64/Prism.Scripting/`
- ExampleApp output targets `PrismEditor/Assets/Scripts/`
- Post-build copies Prism.dll and nethost DLLs into the Editor output directory

## Project Architecture

### Three Workplaces (premake5.lua)

**1. Prism (C++17, Visual Studio)** — Core engine + Editor
- `Prism` — SharedLib (DLL): all engine core, rendering, platform, scripting interop
- `PrismEditor` — ConsoleApp: ImGui-based editor application

**2. PrismManaged (.NET 9, C#)** — Managed scripting layer
- `Prism.Scripting` — SharedLib: C# API for scripts (Input, Time, Math, Renderer, Scene/ECS wrappers)

**3. Examples (.NET 9, C#)** — User scripts
- `ExampleApp` — SharedLib: user-written scripts loaded at runtime by the engine

### Engine Pipeline Flow
```
EntryPoint::main()
  → Application::Run()          (main loop)
    → LayerStack OnUpdate()      (per-frame logic)
    → ImGuiLayer rendering        (editor overlay)
    → RenderCommandQueue::Execute()  (multi-threaded render commands)
```

### Key Modules

| Module | Path | Purpose |
|--------|------|---------|
| Core | `Prism/src/Prism/Core/` | Application, Window, Layer, Log, Input, Time, UUID, Ref |
| Events | `Prism/src/Prism/Events/` | Event system (dispatch, categories) |
| Renderer | `Prism/src/Prism/Renderer/` | Abstraction layer: RenderPass, Material, Mesh, Texture, Shader, SceneRenderer |
| Renderer/OpenGL | `Prism/src/Platform/OpenGL/` | Full OpenGL backend (buffers, shaders, textures, FBO, SSBO, state cache) |
| Scene/ECS | `Prism/src/Prism/Scene/` | Scene management, entt-based ECS (7 component types) |
| Scripting | `Prism/src/Scripting/` | C++/C# interop engine (Rolky-based, InternalCall registration) |
| Editor | `PrismEditor/src/` | ImGui application with EditorLayer, hierarchy panel, property panel |
| C# API | `Prism.Scripting/src/Prism/` | C# counterparts for engine API |

### Render Architecture
- `Renderer::Submit()` pushes lambda commands to a `RenderCommandQueue`
- The main thread drains the queue in `WaitAndRender()`, calling OpenGL backends
- Material system: base Material with runtime MaterialInstance
- Shader system: Prism Shader Language (PSL) — custom format with Properties, RenderCommand blocks, SubShader/Pass, embedded GLSL
- Pipeline features: PBR (metal/roughness), HDR, MSAA, Compute Shader, SSBO, Stencil Buffer Outline

### ECS Components (entt)
- IDComponent, TagComponent, TransformComponent, MeshComponent, ScriptComponent, CameraComponent, SpriteRendererComponent

### C# Scripting
- Uses Rolky for managed/native interop (reflection + InternalCall)
- Entity lifecycle: `OnCreate()` / `OnUpdate()`
- C# side mirrors C++ API: `Prism.Input`, `Prism.Time`, `Prism.Log`, `Prism.Entity`, `Prism.Component`
- Public fields on C# scripts auto-exposed via reflection

### Key Patterns
- **Intrusive ref-counted pointers**: `Ref<T>` / `RefCounted` (custom, not std::shared_ptr)
- **Layer system**: `Layer::OnAttach/OnDetach/OnUpdate/OnImGuiRender/OnEvent`
- **Editor states**: `SceneState::Edit | Play | Pause`
- **Platform defines**: `PR_PLATFORM_WINDOWS`, `PR_DEBUG`, `PR_RELEASE`, `PR_DIST`
- **DLL export**: Classes marked with `PR_API` macro for DLL export/import

### Key Vendors
GLFW, Glad, ImGui (+ImGuizmo), GLM, spdlog, entt (header-only), Assimp, yaml-cpp, stb_image, FastNoise, Rolky (interop), nethost (.NET hosting)
"" 
# AGENTS.md

Behavioral guidelines to reduce common LLM coding mistakes. Merge with project-specific instructions as needed.

**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:
```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.
