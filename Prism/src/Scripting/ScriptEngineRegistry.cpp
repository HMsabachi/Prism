#include "prpch.h"
#include "ScriptEngineRegistry.h"
#include "ScriptEngine.h"
#include "ScriptWrappers.h"
#include <coreclr_delegates.h>
#include <Rolky/Assembly.hpp>
#include <spdlog/fmt/fmt.h>

#include "Native/String.h"
#include "Prism/Utilities/TypeInfo.h"
namespace Prism
{
    std::unordered_map<Rolky::TypeId, std::function<void(Entity&)>> s_CreateComponentFuncs;
    std::unordered_map<Rolky::TypeId, std::function<bool(Entity&)>> s_HasComponentFuncs;

    template<typename TComponent>
    static void RegisterManagedComponent(Rolky::ManagedAssembly& coreAssembly)
    {
        const TypeNameString& componentTypeName = TypeInfo<TComponent, true>().Name();
        std::string componentName = "Prism.";
        componentName += componentTypeName;
        auto& type = coreAssembly.GetType(componentName);
        if (type)
        {
            s_CreateComponentFuncs[type.GetTypeId()] = [](Entity& entity) { entity.AddComponent<TComponent>(); };
            s_HasComponentFuncs[type.GetTypeId()] = [](Entity& entity) { return entity.HasComponent<TComponent>(); };
        }
        else
        {
            PR_CORE_ERROR("No C# component class found for {0}!", componentName);
        }
    }

    static void InitComponentTypes()
    {
        auto& engineAssembly = ScriptEngine::GetEngineAssembly();
        RegisterManagedComponent<TagComponent>(engineAssembly);
        RegisterManagedComponent<TransformComponent>(engineAssembly);
        RegisterManagedComponent<MeshComponent>(engineAssembly);
        RegisterManagedComponent<ScriptComponent>(engineAssembly);
        RegisterManagedComponent<CameraComponent>(engineAssembly);
        RegisterManagedComponent<SpriteRendererComponent>(engineAssembly);
        RegisterManagedComponent<MaterialComponent>(engineAssembly);
        RegisterManagedComponent<RigidBody2DComponent>(engineAssembly);
        RegisterManagedComponent<BoxCollider2DComponent>(engineAssembly);
        RegisterManagedComponent<CircleCollider2DComponent>(engineAssembly);
    }

    void ScriptEngineRegistry::RegisterAll()
    {
        InitComponentTypes();
        auto& engineAssembly = ScriptEngine::GetEngineAssembly();
#define PR_ADD_INTERNAL_CALL(func) engineAssembly.AddInternalCall("Prism.InternalCalls", #func, (void*)&func)
        using namespace Script;
        // Log
        PR_ADD_INTERNAL_CALL(Prism_Log_LogMessage);
        // Time
        PR_ADD_INTERNAL_CALL(Prism_Time_GetDeltaTime);
        PR_ADD_INTERNAL_CALL(Prism_Time_GetUnscaledDeltaTime);
        PR_ADD_INTERNAL_CALL(Prism_Time_GetTime);
        PR_ADD_INTERNAL_CALL(Prism_Time_GetUnscaledTime);
        PR_ADD_INTERNAL_CALL(Prism_Time_GetFixedDeltaTime);
        PR_ADD_INTERNAL_CALL(Prism_Time_GetFrameCount);
        PR_ADD_INTERNAL_CALL(Prism_Time_SetTimeScale);
        PR_ADD_INTERNAL_CALL(Prism_Time_GetTimeScale);
        // Math
        PR_ADD_INTERNAL_CALL(Prism_Noise_PerlinNoise);
        // Input
        PR_ADD_INTERNAL_CALL(Prism_Input_IsKeyPressed);
        // Entity
        PR_ADD_INTERNAL_CALL(Prism_Entity_GetTransform);
        PR_ADD_INTERNAL_CALL(Prism_Entity_SetTransform);
        PR_ADD_INTERNAL_CALL(Prism_Entity_CreateComponent);
        PR_ADD_INTERNAL_CALL(Prism_Entity_HasComponent);
        // Mesh
        PR_ADD_INTERNAL_CALL(Prism_MeshComponent_GetMesh);
        PR_ADD_INTERNAL_CALL(Prism_MeshComponent_SetMesh);
        PR_ADD_INTERNAL_CALL(Prism_Mesh_Constructor);
        PR_ADD_INTERNAL_CALL(Prism_Mesh_Destructor);
        PR_ADD_INTERNAL_CALL(Prism_Mesh_GetMaterial);
        PR_ADD_INTERNAL_CALL(Prism_Mesh_GetMaterialByIndex);
        PR_ADD_INTERNAL_CALL(Prism_Mesh_GetMaterialCount);
        PR_ADD_INTERNAL_CALL(Prism_MeshFactory_CreatePlane);
        // MaterialComponent
        PR_ADD_INTERNAL_CALL(Prism_MaterialComponent_GetMaterial);
        PR_ADD_INTERNAL_CALL(Prism_MaterialComponent_SetMaterial);
        // RigidBody2DComponent
        PR_ADD_INTERNAL_CALL(Prism_RigidBody2DComponent_ApplyLinearImpulse);
        PR_ADD_INTERNAL_CALL(Prism_RigidBody2DComponent_GetLinearVelocity);
        PR_ADD_INTERNAL_CALL(Prism_RigidBody2DComponent_SetLinearVelocity);

        // Entity
        PR_ADD_INTERNAL_CALL(Prism_Entity_FindEntityByTag);
        // Texture2D
        PR_ADD_INTERNAL_CALL(Prism_Texture2D_Constructor);
        PR_ADD_INTERNAL_CALL(Prism_Texture2D_Destructor);
        PR_ADD_INTERNAL_CALL(Prism_Texture2D_SetData);
        // Material
        PR_ADD_INTERNAL_CALL(Prism_Material_Constructor);
        PR_ADD_INTERNAL_CALL(Prism_Material_Destructor);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetFloat);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetTexture);
        PR_ADD_INTERNAL_CALL(Prism_MaterialInstance_Constructor);
        PR_ADD_INTERNAL_CALL(Prism_MaterialInstance_Destructor);
        PR_ADD_INTERNAL_CALL(Prism_MaterialInstance_SetFloat);
        PR_ADD_INTERNAL_CALL(Prism_MaterialInstance_SetVector3);
        PR_ADD_INTERNAL_CALL(Prism_MaterialInstance_SetVector4);
        PR_ADD_INTERNAL_CALL(Prism_MaterialInstance_SetTexture);


        engineAssembly.UploadInternalCalls();

    }
}
