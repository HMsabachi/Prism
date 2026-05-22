#include "prpch.h"
#include "PythonScriptEngine.h"
#include "PythonObject.h"
#include "PythonScriptWrappers.h"
#include "PythonPublicField.h"

#include <Python.h>

#include "Prism/Scene/Components.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"

#include <filesystem>
#include <imgui.h>
#include <functional>

namespace Prism
{
    std::unordered_map<std::string, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
    std::unordered_map<std::string, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;

#define REGISTER_PYTHON_COMPONENT(T, Name) \
    s_PythonCreateComponentFuncs[Name] = [](Entity& entity) { entity.AddComponent<T>(); }; \
    s_PythonHasComponentFuncs[Name] = [](Entity& entity) { return entity.HasComponent<T>(); };

    static void RegisterPythonComponentTypes()
    {
        REGISTER_PYTHON_COMPONENT(TagComponent,            "TagComponent");
        REGISTER_PYTHON_COMPONENT(TransformComponent,       "TransformComponent");
        REGISTER_PYTHON_COMPONENT(MeshComponent,            "MeshComponent");
        REGISTER_PYTHON_COMPONENT(CameraComponent,          "CameraComponent");
        REGISTER_PYTHON_COMPONENT(SpriteRendererComponent,  "SpriteRendererComponent");
        REGISTER_PYTHON_COMPONENT(MaterialComponent,        "MaterialComponent");
        REGISTER_PYTHON_COMPONENT(RigidBody2DComponent,     "RigidBody2DComponent");
        REGISTER_PYTHON_COMPONENT(BoxCollider2DComponent,   "BoxCollider2DComponent");
        REGISTER_PYTHON_COMPONENT(CircleCollider2DComponent,"CircleCollider2DComponent");
        REGISTER_PYTHON_COMPONENT(RigidBodyComponent,       "RigidBodyComponent");
        REGISTER_PYTHON_COMPONENT(BoxColliderComponent,     "BoxColliderComponent");
        REGISTER_PYTHON_COMPONENT(SphereColliderComponent,  "SphereColliderComponent");
        REGISTER_PYTHON_COMPONENT(CapsuleColliderComponent, "CapsuleColliderComponent");
    }

#undef REGISTER_PYTHON_COMPONENT
}

namespace Prism
{
    PythonScriptEngine::~PythonScriptEngine()
    {
        Shutdown();
    }

    bool PythonScriptEngine::Initialize()
    {
        PR_PROFILE_FUNCTION();

        if (m_Initialized)
            return true;

        if (!Python::ScriptHost::Initialize())
        {
            PR_CORE_ERROR("[Python] 初始化解释器失败！");
            return false;
        }

        RegisterPythonComponentTypes();
        Script::RegisterPrismModule();

        m_Initialized = true;
        PR_CORE_TRACE("[Python] Python 运行时已初始化");
        return true;
    }

    void PythonScriptEngine::Shutdown()
    {
        PR_PROFILE_FUNCTION();

        if (!m_Initialized)
            return;

        Python::ScriptHost::Shutdown();
        m_Initialized = false;
        m_SceneContext = nullptr;
        PR_CORE_INFO("[Python] Python 解释器已关闭");
    }

    bool PythonScriptEngine::LoadEngineAssembly(const std::string& path)
    {
        PR_CORE_INFO("[Python] LoadEngineAssembly({0}) — Python 引擎不需要加载程序集", path);
        return true;
    }

    bool PythonScriptEngine::LoadAppAssembly(const std::string& path)
    {
        PR_CORE_INFO("[Python] LoadAppAssembly({0}) — 脚本目录已在 sys.path 中", path);
        return true;
    }

    void PythonScriptEngine::ReloadAssembly(const std::string& path)
    {
        PR_PROFILE_FUNCTION();
        PR_CORE_INFO("[Python] ReloadAssembly({0}) — 暂未实现", path);
    }

    void PythonScriptEngine::SetSceneContext(const Ref<Scene>& scene)
    {
        m_SceneContext = scene;
    }

    const Ref<Scene>& PythonScriptEngine::GetCurrentSceneContext()
    {
        return m_SceneContext;
    }

    bool PythonScriptEngine::ModuleExists(const std::string& moduleName)
    {
        PR_PROFILE_FUNCTION();
        return Python::ScriptModule::ModuleExists(moduleName.c_str());
    }

    void PythonScriptEngine::InitScriptEntity(Entity& entity, ScriptGroup& group)
    {
        PR_PROFILE_FUNCTION();

        if (group.ModuleName.empty())
            return;

        auto mod = Python::ScriptModule::Import(group.ModuleName.c_str());
        if (!mod.IsValid())
        {
            PR_CORE_ERROR("[Python] 无法导入模块 '{0}'", group.ModuleName);
            return;
        }
        auto scriptClass = Python::ScriptClass::From(mod, group.ModuleName.c_str());
        if (!scriptClass.IsValid())
        {
            PR_CORE_ERROR("[Python] 在模块 '{0}' 中找不到类 '{0}'", group.ModuleName);
            return;
        }

        // Detect methods
        group.MethodMask = 0;
        if (scriptClass.HasMethod("OnCreate"))     group.MethodMask |= BIT(0);
        if (scriptClass.HasMethod("OnUpdate"))     group.MethodMask |= BIT(1);
        if (scriptClass.HasMethod("OnFixedUpdate")) group.MethodMask |= BIT(2);
        if (scriptClass.HasMethod("OnDestroy"))    group.MethodMask |= BIT(3);

        // Save old fields
        std::unordered_map<std::string, std::unique_ptr<PublicField>> oldFields;
        for (auto& [name, field] : group.Fields)
            oldFields[name] = std::move(field);
        group.Fields.clear();

        // Discover public fields via __annotations__
        auto annotations = scriptClass.GetAnnotations();
        if (!annotations.empty())
        {
            auto tempObj = scriptClass.CreateInstance();

            for (auto& [fieldName, annoRef] : annotations)
            {
                PyObject* pyType = reinterpret_cast<PyObject*>(annoRef.Get());
                FieldType fieldType = FieldType::None;

                if (pyType == (PyObject*)&PyFloat_Type)
                    fieldType = FieldType::Float;
                else if (pyType == (PyObject*)&PyLong_Type)
                    fieldType = FieldType::Int;
                // String skipped: field storage only supports trivially-copyable types

                if (fieldType == FieldType::None)
                    continue;

                if (oldFields.find(fieldName) != oldFields.end())
                {
                    group.Fields[fieldName] = std::move(oldFields.at(fieldName));
                }
                else if (tempObj.IsValid())
                {
                    auto field = std::make_unique<PythonPublicField>(fieldName, fieldType, &group);
                    uint8_t defaultBuf[32] = {};
                    tempObj.GetFieldRaw(fieldName.c_str(), defaultBuf);
                    field->SetStoredValueRaw(defaultBuf);
                    group.Fields[fieldName] = std::move(field);
                }
            }
        }
    }

    void PythonScriptEngine::ShutdownScriptEntity(ScriptGroup& group)
    {
        group.Instance.reset(); // PythonObject 析构时自动调 OnDestroy
    }

    void PythonScriptEngine::InstantiateEntityClass(ScriptGroup& group)
    {
        PR_PROFILE_FUNCTION();

        // Need to re-import module and get class each time (Python objects may be stale)
        auto mod = Python::ScriptModule::Import(group.ModuleName.c_str());
        if (!mod.IsValid()) return;
        auto scriptClass = Python::ScriptClass::From(mod, group.ModuleName.c_str());
        if (!scriptClass.IsValid()) return;

        auto pyObj = scriptClass.CreateInstance();
        auto pythonObj = std::make_unique<PythonObject>(std::move(pyObj));
        pythonObj->GetObject().SetField<uint64_t>("EntityID", (uint64_t)group.EntityID);

        group.Instance = std::move(pythonObj);

        // Copy stored field values to runtime
        for (auto& [name, field] : group.Fields)
            field->CopyStoredValueToRuntime();
    }

    void PythonScriptEngine::OnImGuiRender()
    {
        ImGui::Begin("Python Script Engine");
        ImGui::Text("Initialized: %s", m_Initialized ? "Yes" : "No");
        ImGui::End();
    }

}
