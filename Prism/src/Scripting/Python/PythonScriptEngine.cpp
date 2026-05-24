#include "prpch.h"
#include "PythonScriptEngine.h"
#include "PythonScriptStorage.h"
#include "PythonScriptWrappers.h"
#include "PythonScriptMetaRegistry.h"

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
    // Static members
    Ref<Scene> PythonScriptEngine::s_SceneContext;
    bool PythonScriptEngine::s_Initialized = false;
    std::unordered_map<UUID, std::unordered_map<UUID, Python::ScriptObject>> PythonScriptEngine::s_PythonScriptObjects;

    void PythonScriptEngine::Initialize()
    {
        PR_PROFILE_FUNCTION();

        if (s_Initialized)
            return;

        if (!Python::ScriptHost::Initialize())
        {
            PR_CORE_ERROR("[Python] 初始化解释器失败！");
            return;
        }

        RegisterPythonComponentTypes();
        Script::RegisterPrismModule();

        s_Initialized = true;
        PR_CORE_TRACE("[Python] Python 运行时已初始化");

        PythonScriptMetaRegistry::BuildCache();
    }

    void PythonScriptEngine::Shutdown()
    {
        PR_PROFILE_FUNCTION();

        if (!s_Initialized)
            return;

        PythonScriptMetaRegistry::Shutdown();
        Python::ScriptHost::Shutdown();
        s_Initialized = false;
        s_SceneContext = nullptr;
        PR_CORE_INFO("[Python] Python 解释器已关闭");
    }

    PythonEntityScriptStorage& PythonScriptEngine::GetEntityScriptStorage(PythonScriptStorage& storage, UUID scriptID)
    {
        auto it = storage.EntityStorage.find(scriptID);
        PR_CORE_ASSERT(it != storage.EntityStorage.end(), "PythonScript entity not found!");
        return it->second;
    }

    void PythonScriptEngine::SetSceneContext(const Ref<Scene>& scene)
    {
        s_SceneContext = scene;
    }

    const Ref<Scene>& PythonScriptEngine::GetCurrentSceneContext()
    {
        return s_SceneContext;
    }

    Python::ScriptObject* PythonScriptEngine::GetScriptObject(UUID sceneID, UUID scriptID)
    {
        auto sceneIt = s_PythonScriptObjects.find(sceneID);
        if (sceneIt == s_PythonScriptObjects.end())
            return nullptr;
        auto objIt = sceneIt->second.find(scriptID);
        return objIt != sceneIt->second.end() ? &objIt->second : nullptr;
    }

    void PythonScriptEngine::RemoveScriptObject(PythonScriptStorage& storage, UUID scriptID)
    {
        auto sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        auto sceneIt = s_PythonScriptObjects.find(sceneID);
        if (sceneIt != s_PythonScriptObjects.end())
        {
            sceneIt->second.erase(scriptID);
            if (sceneIt->second.empty())
                s_PythonScriptObjects.erase(sceneIt);
            storage.Remove(scriptID);
        }
    }

    void PythonScriptEngine::ReleaseAll()
    {
        s_PythonScriptObjects.clear();
    }
}
