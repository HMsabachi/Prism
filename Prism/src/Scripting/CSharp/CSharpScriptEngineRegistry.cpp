#include "prpch.h"
#include "CSharpScriptEngineRegistry.h"
#include "CSharpScriptEngine.h"
#include "CSharpScriptWrappers.h"
#include <coreclr_delegates.h>
#include <Rolky/Assembly.hpp>
#include <spdlog/fmt/fmt.h>

#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"

#include "Prism/Utilities/TypeInfo.h"

namespace Prism
{
    std::unordered_map<Rolky::TypeId, std::function<void(Entity&)>> s_CreateComponentFuncs;
    std::unordered_map<Rolky::TypeId, std::function<Rolky::Bool32(Entity&)>> s_HasComponentFuncs;

    template<typename TComponent>
    static void RegisterManagedComponent()
    {
        const TypeNameString& componentTypeName = TypeInfo<TComponent, true>().Name();
        std::string componentName = "Prism.";
        componentName += componentTypeName;
        auto& engineAssembly = CSharpScriptEngine::GetEngineAssembly();
        auto& type = engineAssembly.GetLocalType(componentName);
        if (type)
        {
            s_CreateComponentFuncs[type.GetTypeId()] = [](Entity& entity) { entity.AddComponent<TComponent>(); };
            s_HasComponentFuncs[type.GetTypeId()] = [](Entity& entity) { return (Rolky::Bool32)entity.HasComponent<TComponent>(); };
        }
        else
        {
            PR_CORE_ERROR("No C# component class found for {0}!", componentName);
        }
    }

    static void InitComponentTypes()
    {
        RegisterManagedComponent<TagComponent>();
        RegisterManagedComponent<TransformComponent>();
        RegisterManagedComponent<MeshRendererComponent>();
        RegisterManagedComponent<CameraComponent>();
        RegisterManagedComponent<SpriteRendererComponent>();
        RegisterManagedComponent<RigidBody2DComponent>();
        RegisterManagedComponent<BoxCollider2DComponent>();
        RegisterManagedComponent<CircleCollider2DComponent>();
        RegisterManagedComponent<RigidBodyComponent>();
        RegisterManagedComponent<BoxColliderComponent>();
        RegisterManagedComponent<SphereColliderComponent>();
        RegisterManagedComponent<CapsuleColliderComponent>();
    }

    void CSharpScriptEngineRegistry::RegisterAll()
    {
        s_CreateComponentFuncs.clear();
        s_HasComponentFuncs.clear();
        InitComponentTypes();
        auto& engineAssembly = CSharpScriptEngine::GetEngineAssembly();
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
        PR_ADD_INTERNAL_CALL(Prism_Time_SetFixedDeltaTime);
        // Math
        PR_ADD_INTERNAL_CALL(Prism_Noise_PerlinNoise);
        // Input
        PR_ADD_INTERNAL_CALL(Prism_Input_IsKeyPressed);
        PR_ADD_INTERNAL_CALL(Prism_Input_GetMousePosition);
        PR_ADD_INTERNAL_CALL(Prism_Input_SetCursorMode);
        PR_ADD_INTERNAL_CALL(Prism_Input_GetCursorMode);
        PR_ADD_INTERNAL_CALL(Prism_Input_IsMouseButtonPressed);
        // Entity
        PR_ADD_INTERNAL_CALL(Prism_Entity_GetTransform);
        PR_ADD_INTERNAL_CALL(Prism_Entity_SetTransform);
        PR_ADD_INTERNAL_CALL(Prism_Entity_CreateComponent);
        PR_ADD_INTERNAL_CALL(Prism_Entity_HasComponent);
        PR_ADD_INTERNAL_CALL(Prism_Entity_FindEntityByTag);
        PR_ADD_INTERNAL_CALL(Prism_Entity_AddBehaviour);
        PR_ADD_INTERNAL_CALL(Prism_Entity_RemoveBehaviour);
        PR_ADD_INTERNAL_CALL(Prism_Entity_GetBehaviour);
        PR_ADD_INTERNAL_CALL(Prism_Behaviour_GetEnabled);
        PR_ADD_INTERNAL_CALL(Prism_Behaviour_SetEnabled);
        // MeshRendererComponent
        PR_ADD_INTERNAL_CALL(Prism_MeshRendererComponent_GetMesh);
        PR_ADD_INTERNAL_CALL(Prism_MeshRendererComponent_SetMesh);
        PR_ADD_INTERNAL_CALL(Prism_MeshRendererComponent_GetMaterial);
        PR_ADD_INTERNAL_CALL(Prism_MeshRendererComponent_SetMaterial);
        PR_ADD_INTERNAL_CALL(Prism_MeshRendererComponent_GetMaterialCount);
        PR_ADD_INTERNAL_CALL(Prism_MeshRendererComponent_GetMaterials);
        PR_ADD_INTERNAL_CALL(Prism_MeshRendererComponent_SetMaterials);
        // Mesh
        PR_ADD_INTERNAL_CALL(Prism_Mesh_Constructor);
        PR_ADD_INTERNAL_CALL(Prism_Mesh_Destructor);
        PR_ADD_INTERNAL_CALL(Prism_MeshFactory_CreatePlane);
        // RigidBody2DComponent
        PR_ADD_INTERNAL_CALL(Prism_RigidBody2DComponent_ApplyLinearImpulse);
        PR_ADD_INTERNAL_CALL(Prism_RigidBody2DComponent_GetLinearVelocity);
        PR_ADD_INTERNAL_CALL(Prism_RigidBody2DComponent_SetLinearVelocity);
        // TransformComponent
        PR_ADD_INTERNAL_CALL(Prism_TransformComponent_GetPosition);
        PR_ADD_INTERNAL_CALL(Prism_TransformComponent_GetRotation);
        PR_ADD_INTERNAL_CALL(Prism_TransformComponent_GetScale);
        PR_ADD_INTERNAL_CALL(Prism_TransformComponent_SetPosition);
        PR_ADD_INTERNAL_CALL(Prism_TransformComponent_SetRotation);
        PR_ADD_INTERNAL_CALL(Prism_TransformComponent_SetScale);
        // RigidBodyComponent
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_AddForce);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_AddTorque);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_GetLinearVelocity);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_SetLinearVelocity);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_Rotate);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_GetLayer);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_GetMass);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_SetMass);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_GetBodyType);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_GetAngularVelocity);
        PR_ADD_INTERNAL_CALL(Prism_RigidBodyComponent_SetAngularVelocity);
        // Physics
        PR_ADD_INTERNAL_CALL(Prism_Physics_Raycast);
        PR_ADD_INTERNAL_CALL(Prism_Physics_OverlapBox);
        PR_ADD_INTERNAL_CALL(Prism_Physics_OverlapSphere);
        PR_ADD_INTERNAL_CALL(Prism_Physics_OverlapCapsule);
        PR_ADD_INTERNAL_CALL(Prism_Physics_OverlapBoxNonAlloc);
        PR_ADD_INTERNAL_CALL(Prism_Physics_OverlapCapsuleNonAlloc);
        PR_ADD_INTERNAL_CALL(Prism_Physics_OverlapSphereNonAlloc);
        PR_ADD_INTERNAL_CALL(Prism_Physics_GetGravity);
        PR_ADD_INTERNAL_CALL(Prism_Physics_SetGravity);
        // Texture2D
        PR_ADD_INTERNAL_CALL(Prism_Texture2D_Constructor);
        PR_ADD_INTERNAL_CALL(Prism_Texture2D_Destructor);
        PR_ADD_INTERNAL_CALL(Prism_Texture2D_SetData);
        // Material
        PR_ADD_INTERNAL_CALL(Prism_Material_Constructor);
        PR_ADD_INTERNAL_CALL(Prism_Material_GetDefaultMaterial);
        PR_ADD_INTERNAL_CALL(Prism_Material_Destructor);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetFloat);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetInt);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetBool);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetColor3);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetColor);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetMatrix4);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetVector2);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetVector3);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetVector4);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetTexture);
        PR_ADD_INTERNAL_CALL(Prism_Material_SetKeyword);
        PR_ADD_INTERNAL_CALL(Prism_Material_IsKeywordEnabled);

        engineAssembly.UploadInternalCalls();
    }
}
