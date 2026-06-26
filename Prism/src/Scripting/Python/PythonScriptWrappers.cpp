#include "prpch.h"
#include "PythonScriptWrappers.h"

#include "Prism/Core/Input.h"
#include "Prism/Core/KeyCodes.h"
#include "Prism/Core/Math/Noise.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include "Prism/Physics/PXPhysicsWrappers.h"
#include "Prism/Asset/ModelImporter.h"
#include "Prism/Physics/Physics.h"

#include "Scripting/Python/PythonScriptEngine.h"
#include "Prism/Scene/Systems/ScriptSystem.h"
#include "Scripting/Python/PythonScriptEngineRegistry.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"
#include "Scripting/Python/Interop/PythonMathBridge.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <box2d/box2d.h>
#include <PhysX/PxPhysicsAPI.h>
#include "Prism/Physics/PhysicsUtil.h"

#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Renderer.h"

namespace Prism::Script
{

    static Entity GetEntityFromEntityID(uint64_t entityID)
    {
        WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
        PR_CORE_ASSERT(scene, "没有激活的场景！");
        const auto& entityMap = scene->GetEntityMap();
        PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "无效的实体 ID！");
        return entityMap.at(entityID);
    }

#pragma region Log

    Python::ScriptValue* Prism_Log_LogMessage(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        LogLevel level = static_cast<LogLevel>(Python::ValueToInt(Python::GetTupleElement(argsRef, 0)));
        std::string message = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        message = "[Python] " + message;

        switch (level)
        {
        case LogLevel::Trace:	PR_CORE_TRACE(message); break;
        case LogLevel::Debug:	PR_CORE_INFO(message);  break;
        case LogLevel::Info:	PR_CORE_INFO(message);  break;
        case LogLevel::Warn:	PR_CORE_WARN(message);  break;
        case LogLevel::Error:	PR_CORE_ERROR(message); break;
        case LogLevel::Critical:PR_CORE_FATAL(message); break;
        }
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region Time

    Python::ScriptValue* Prism_Time_GetDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::FloatToValue(Time::GetDeltaTime()).Detach();
    }

    Python::ScriptValue* Prism_Time_GetTime(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::FloatToValue(Time::GetTime()).Detach();
    }

    Python::ScriptValue* Prism_Time_GetUnscaledDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::FloatToValue(Time::GetUnscaledDeltaTime()).Detach();
    }

    Python::ScriptValue* Prism_Time_GetUnscaledTime(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::FloatToValue(Time::GetUnscaledTime()).Detach();
    }

    Python::ScriptValue* Prism_Time_GetFixedDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::FloatToValue(Time::GetFixedDeltaTime()).Detach();
    }

    Python::ScriptValue* Prism_Time_GetFrameCount(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::UInt64ToValue(static_cast<uint64_t>(Time::GetFrameCount())).Detach();
    }

    Python::ScriptValue* Prism_Time_GetTimeScale(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::FloatToValue(Time::GetTimeScale()).Detach();
    }

    Python::ScriptValue* Prism_Time_SetTimeScale(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        float scale = Python::ValueToFloat(Python::GetTupleElement(argsRef, 0));
        Time::SetTimeScale(scale);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Time_SetFixedDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        float fixedDeltaTime = Python::ValueToFloat(Python::GetTupleElement(argsRef, 0));
        Time::SetFixedDeltaTime(fixedDeltaTime);
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region Math

    Python::ScriptValue* Prism_Noise_PerlinNoise(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        float x = Python::ValueToFloat(Python::GetTupleElement(argsRef, 0));
        float y = Python::ValueToFloat(Python::GetTupleElement(argsRef, 1));
        return Python::FloatToValue(Noise::PerlinNoise(x, y)).Detach();
    }

#pragma endregion

#pragma region Input

    Python::ScriptValue* Prism_Input_IsKeyPressed(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        KeyCode key = static_cast<KeyCode>(Python::ValueToInt(Python::GetTupleElement(argsRef, 0)));
        return Python::BoolToValue(Input::IsKeyPressed(key)).Detach();
    }

    Python::ScriptValue* Prism_Input_GetMousePosition(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        auto [x, y] = Input::GetMousePosition();
        return Python::Vec2ToValue(glm::vec2(x, y)).Detach();
    }

    Python::ScriptValue* Prism_Input_SetCursorMode(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        CursorMode mode = static_cast<CursorMode>(Python::ValueToInt(Python::GetTupleElement(argsRef, 0)));
        Input::SetCursorMode(mode);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Input_IsMouseButtonPressed(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        MouseButton button = static_cast<MouseButton>(Python::ValueToInt(Python::GetTupleElement(argsRef, 0)));
        return Python::BoolToValue(Input::IsMouseButtonPressed(button)).Detach();
    }

    Python::ScriptValue* Prism_Input_GetCursorMode(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::IntToValue(static_cast<int>(Input::GetCursorMode())).Detach();
    }

#pragma endregion

#pragma region Entity



    Python::ScriptValue* Prism_Entity_CreateComponent(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));

        // 接收 Python 类对象，通过 ScriptClass::GetTypeId() 获取类型 ID
        Python::ScriptRef classObj = Python::GetTupleElement(argsRef, 1);
        Python::ScriptClass cls = Python::ScriptClass::FromRef(classObj);
        uint64_t typeId = cls.GetTypeId();

        auto it = s_PythonCreateComponentFuncs.find(typeId);
        if (it != s_PythonCreateComponentFuncs.end())
            it->second(entity);

        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Entity_HasComponent(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));

        // 接收 Python 类对象
        Python::ScriptRef classObj = Python::GetTupleElement(argsRef, 1);
        Python::ScriptClass cls = Python::ScriptClass::FromRef(classObj);
        uint64_t typeId = cls.GetTypeId();

        auto it = s_PythonHasComponentFuncs.find(typeId);
        if (it != s_PythonHasComponentFuncs.end())
            return Python::BoolToValue(it->second(entity)).Detach();

        return Python::BoolToValue(false).Detach();
    }

    Python::ScriptValue* Prism_Entity_AddBehaviour(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptClass cls = Python::ScriptClass::FromRef(Python::GetTupleElement(argsRef, 1));
        PR_CORE_ASSERT(cls.IsValid(), "Python class not found!");

        UUID classID = PythonScriptMetaRegistry::GenerateClassID(cls.GetFullName());
        auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
        UUID behaviourID = ss->AddPythonBehaviour(entity, classID);

        UUID sceneID = PythonScriptEngine::GetCurrentSceneContext()->GetUUID();
        auto* obj = PythonScriptEngine::GetScriptObject(sceneID, behaviourID);
        PR_CORE_ASSERT(obj && obj->IsValid(), "Failed to get created behaviour instance!");
        return obj->GetRef().Detach();
    }

    Python::ScriptValue* Prism_Entity_FindEntityByTag(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        std::string tag = Python::ValueToString(Python::GetTupleElement(argsRef, 0));

        WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
        Entity entity = scene->FindEntityByTag(tag);
        return Python::UInt64ToValue(entity ? (uint64_t)entity.GetUUID() : (uint64_t)0).Detach();
    }

    Python::ScriptValue* Prism_Entity_RemoveBehaviour(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        UUID behaviourID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 1)));
        auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
        ss->RemovePythonBehaviour(entity, behaviourID);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Entity_GetBehaviour(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptClass cls = Python::ScriptClass::FromRef(Python::GetTupleElement(argsRef, 1));
        UUID classID = PythonScriptMetaRegistry::GenerateClassID(cls.GetFullName());

        auto& comp = entity.GetComponent<PythonScriptComponent>();
        UUID sceneID = PythonScriptEngine::GetCurrentSceneContext()->GetUUID();
        for (auto& [bid, binding] : comp.Behaviours)
        {
            if (binding.ClassID == classID)
            {
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj)
                    return obj->GetRef().Detach();
            }
        }
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_GetRotation(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Transform& transform = entity.Transformation();
        glm::vec3 euler = transform.GetRotation();

        return Python::Vec3ToValue(euler).Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_SetRotation(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);

        glm::vec3 euler = Python::ValueToVec3(vecObj);
        Transform& transform = entity.Transformation();
        transform.SetRotation(euler);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_GetScale(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Transform& transform = entity.Transformation();

        return Python::Vec3ToValue(transform.GetScale()).Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_SetScale(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);

        Transform& transform = entity.Transformation();
        transform.SetScale(Python::ValueToVec3(vecObj));
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_GetTransform(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Transform& transform = entity.Transformation();

        Python::ScriptRef data[3];
        data[0] = Python::Vec3ToValue(transform.GetPosition());
        data[1] = Python::Vec3ToValue(transform.GetRotation());
        data[2] = Python::Vec3ToValue(transform.GetScale());
        return Python::MakeTuple(data, 3).Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_SetTransform(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        glm::vec3 position = Python::ValueToVec3(Python::GetTupleElement(argsRef, 1));
        glm::vec3 rotation = Python::ValueToVec3(Python::GetTupleElement(argsRef, 2));
        glm::vec3 scale = Python::ValueToVec3(Python::GetTupleElement(argsRef, 3));

        Transform& transform = entity.Transformation();
        transform.SetPosition(position);
        transform.SetRotation(rotation);
        transform.SetScale(scale);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Behaviour_GetEnabled(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t behaviourID = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
        return Python::BoolToValue(ss->GetEnabled(UUID(behaviourID))).Detach();
    }

    Python::ScriptValue* Prism_Behaviour_SetEnabled(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t behaviourID = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        bool enabled = Python::ValueToBool(Python::GetTupleElement(argsRef, 1));
        auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
        ss->SetEnabled(UUID(behaviourID), enabled);
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region TransformComponent

    Python::ScriptValue* Prism_TransformComponent_GetPosition(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Transform& transform = entity.Transformation();

        return Python::Vec3ToValue(transform.GetPosition()).Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_SetPosition(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);

        Transform& transform = entity.Transformation();
        transform.SetPosition(Python::ValueToVec3(vecObj));
        return Python::NoneValue().Detach();
    }

#pragma region TagComponent

    Python::ScriptValue* Prism_TagComponent_GetTag(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        auto& tc = entity.GetComponent<TagComponent>();
        return Python::StringToValue(tc.Tag).Detach();
    }

    Python::ScriptValue* Prism_TagComponent_SetTag(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        std::string tag = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        entity.GetComponent<TagComponent>().Tag = tag;
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region RigidBody2DComponent

    Python::ScriptValue* Prism_RigidBody2DComponent_ApplyLinearImpulse(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef impulseObj = Python::GetTupleElement(argsRef, 1);
        Python::ScriptRef offsetObj = Python::GetTupleElement(argsRef, 2);
        bool wake = Python::ValueToBool(Python::GetTupleElement(argsRef, 3));

        glm::vec2 impulse = Python::ValueToVec2(impulseObj);
        glm::vec2 offset = Python::ValueToVec2(offsetObj);

        auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
        body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), b2Vec2(offset.x, offset.y), wake);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_RigidBody2DComponent_GetLinearVelocity(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));

        auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
        const b2Vec2& velocity = body->GetLinearVelocity();
        return Python::Vec2ToValue(glm::vec2(velocity.x, velocity.y)).Detach();
    }

    Python::ScriptValue* Prism_RigidBody2DComponent_SetLinearVelocity(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);

        glm::vec2 velocity = Python::ValueToVec2(vecObj);
        auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
        body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region RigidBodyComponent

    Python::ScriptValue* Prism_RigidBodyComponent_AddForce(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef forceObj = Python::GetTupleElement(argsRef, 1);
        int32_t forceMode = Python::ValueToInt(Python::GetTupleElement(argsRef, 2));

        glm::vec3 force = Python::ValueToVec3(forceObj);
        auto& rb = entity.GetComponent<RigidBodyComponent>();
        if (rb.IsKinematic)
        {
            PR_CORE_WARN("Cannot add a force to a kinematic actor! EntityID({0})", (uint64_t)entity.GetUUID());
            return Python::NoneValue().Detach();
        }
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);
        dynamicActor->addForce(physx::PxVec3(force.x, force.y, force.z), (physx::PxForceMode::Enum)forceMode);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_AddTorque(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef torqueObj = Python::GetTupleElement(argsRef, 1);
        int32_t forceMode = Python::ValueToInt(Python::GetTupleElement(argsRef, 2));

        glm::vec3 torque = Python::ValueToVec3(torqueObj);
        auto& rb = entity.GetComponent<RigidBodyComponent>();
        if (rb.IsKinematic)
        {
            PR_CORE_WARN("Cannot add torque to a kinematic actor! EntityID({0})", (uint64_t)entity.GetUUID());
            return Python::NoneValue().Detach();
        }
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);
        dynamicActor->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), (physx::PxForceMode::Enum)forceMode);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_GetLinearVelocity(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));

        auto& rb = entity.GetComponent<RigidBodyComponent>();
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);
        physx::PxVec3 velocity = dynamicActor->getLinearVelocity();
        return Python::Vec3ToValue(glm::vec3(velocity.x, velocity.y, velocity.z)).Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_SetLinearVelocity(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);

        glm::vec3 velocity = Python::ValueToVec3(vecObj);
        auto& rb = entity.GetComponent<RigidBodyComponent>();
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);
        physx::PxVec3 pxVelocity(velocity.x, velocity.y, velocity.z);
        if (!pxVelocity.isFinite())
            return Python::NoneValue().Detach();
        dynamicActor->setLinearVelocity(pxVelocity);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_Rotate(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);
        glm::vec3 rotation = Python::ValueToVec3(vecObj);

        auto& rb = entity.GetComponent<RigidBodyComponent>();
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);

        physx::PxTransform transform = dynamicActor->getGlobalPose();
        transform.q *= (physx::PxQuat(glm::radians(rotation.x), { 1.0F, 0.0F, 0.0F })
            * physx::PxQuat(glm::radians(rotation.y), { 0.0F, 1.0F, 0.0F })
            * physx::PxQuat(glm::radians(rotation.z), { 0.0F, 0.0F, 1.0F }));

        dynamicActor->setGlobalPose(transform);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_GetLayer(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
        auto& component = entity.GetComponent<RigidBodyComponent>();
        return Python::UInt64ToValue(component.Layer).Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_GetMass(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
        auto& component = entity.GetComponent<RigidBodyComponent>();

        physx::PxRigidActor* actor = (physx::PxRigidActor*)component.RuntimeActor;
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);

        return Python::FloatToValue(dynamicActor->getMass()).Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_SetMass(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        float mass = Python::ValueToFloat(Python::GetTupleElement(argsRef, 1));
        PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
        auto& component = entity.GetComponent<RigidBodyComponent>();

        physx::PxRigidActor* actor = (physx::PxRigidActor*)component.RuntimeActor;
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);

        component.Mass = mass;
        physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, mass);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_GetBodyType(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
        auto& component = entity.GetComponent<RigidBodyComponent>();
        return Python::UInt64ToValue((uint64_t)component.BodyType).Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_GetAngularVelocity(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
        auto& component = entity.GetComponent<RigidBodyComponent>();

        physx::PxRigidActor* actor = (physx::PxRigidActor*)component.RuntimeActor;
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);

        physx::PxVec3 velocity = dynamicActor->getAngularVelocity();
        return Python::Vec3ToValue(glm::vec3(velocity.x, velocity.y, velocity.z)).Detach();
    }

    Python::ScriptValue* Prism_RigidBodyComponent_SetAngularVelocity(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        glm::vec3 velocity = Python::ValueToVec3(Python::GetTupleElement(argsRef, 1));
        PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
        auto& component = entity.GetComponent<RigidBodyComponent>();

        physx::PxRigidActor* actor = (physx::PxRigidActor*)component.RuntimeActor;
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        PR_CORE_ASSERT(dynamicActor);

        dynamicActor->setAngularVelocity({ velocity.x, velocity.y, velocity.z });
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region Physics
    Python::ScriptValue* Prism_Physics_Raycast(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        glm::vec3 origin = Python::ValueToVec3(Python::GetTupleElement(argsRef, 0));
        glm::vec3 direction = Python::ValueToVec3(Python::GetTupleElement(argsRef, 1));
        float maxDistance = Python::ValueToFloat(Python::GetTupleElement(argsRef, 2));

        RaycastHit hit;
        if (PXPhysicsWrappers::Raycast(origin, direction, maxDistance, &hit))
        {
            Python::ScriptRef elements[4];
            elements[0] = Python::UInt64ToValue(hit.EntityID);
            elements[1] = Python::Vec3ToValue(hit.Position);
            elements[2] = Python::Vec3ToValue(hit.Normal);
            elements[3] = Python::FloatToValue(hit.Distance);
            return Python::MakeTuple(elements, 4).Detach();
        }

        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Physics_OverlapBox(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        glm::vec3 origin = Python::ValueToVec3(Python::GetTupleElement(argsRef, 0));
        glm::vec3 halfSize = Python::ValueToVec3(Python::GetTupleElement(argsRef, 1));

        std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
        uint32_t count;
        if (PXPhysicsWrappers::OverlapBox(origin, halfSize, buffer, &count))
        {
            Python::ScriptRef* elements = new Python::ScriptRef[count];
            uint32_t arrayIndex = 0;

            for (uint32_t i = 0; i < count; i++)
            {
                Entity& entity = *(Entity*)buffer[i].actor->userData;

                if (entity.HasComponent<BoxColliderComponent>())
                {
                    auto& bc = entity.GetComponent<BoxColliderComponent>();
                    Python::ScriptRef data[9];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(0); // Box
                    data[2] = Python::BoolToValue(bc.IsTrigger);
                    data[3] = Python::FloatToValue(bc.Size.x);
                    data[4] = Python::FloatToValue(bc.Size.y);
                    data[5] = Python::FloatToValue(bc.Size.z);
                    data[6] = Python::FloatToValue(bc.Offset.x);
                    data[7] = Python::FloatToValue(bc.Offset.y);
                    data[8] = Python::FloatToValue(bc.Offset.z);
                    elements[arrayIndex++] = Python::MakeTuple(data, 9);
                }
                else if (entity.HasComponent<SphereColliderComponent>())
                {
                    auto& sc = entity.GetComponent<SphereColliderComponent>();
                    Python::ScriptRef data[4];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(1); // Sphere
                    data[2] = Python::BoolToValue(sc.IsTrigger);
                    data[3] = Python::FloatToValue(sc.Radius);
                    elements[arrayIndex++] = Python::MakeTuple(data, 4);
                }
                else if (entity.HasComponent<CapsuleColliderComponent>())
                {
                    auto& cc = entity.GetComponent<CapsuleColliderComponent>();
                    Python::ScriptRef data[5];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(2); // Capsule
                    data[2] = Python::BoolToValue(cc.IsTrigger);
                    data[3] = Python::FloatToValue(cc.Radius);
                    data[4] = Python::FloatToValue(cc.Height);
                    elements[arrayIndex++] = Python::MakeTuple(data, 5);
                }
                else if (entity.HasComponent<MeshColliderComponent>())
                {
                    auto& mc = entity.GetComponent<MeshColliderComponent>();
                    Python::ScriptRef data[4];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(3); // Mesh
                    data[2] = Python::BoolToValue(mc.IsTrigger);
                    data[3] = Python::UInt64ToValue((uint64_t)new Ref<Mesh>(mc.CollisionMesh));
                    elements[arrayIndex++] = Python::MakeTuple(data, 4);
                }
            }

            Python::ScriptRef tuple = Python::MakeTuple(elements, arrayIndex);
            delete[] elements;
            return tuple.Detach();
        }

        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Physics_OverlapSphere(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        glm::vec3 origin = Python::ValueToVec3(Python::GetTupleElement(argsRef, 0));
        float radius = Python::ValueToFloat(Python::GetTupleElement(argsRef, 1));

        std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
        uint32_t count;
        if (PXPhysicsWrappers::OverlapSphere(origin, radius, buffer, &count))
        {
            Python::ScriptRef* elements = new Python::ScriptRef[count];
            uint32_t arrayIndex = 0;

            for (uint32_t i = 0; i < count; i++)
            {
                Entity& entity = *(Entity*)buffer[i].actor->userData;

                if (entity.HasComponent<BoxColliderComponent>())
                {
                    auto& bc = entity.GetComponent<BoxColliderComponent>();
                    Python::ScriptRef data[9];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(0); // Box
                    data[2] = Python::BoolToValue(bc.IsTrigger);
                    data[3] = Python::FloatToValue(bc.Size.x);
                    data[4] = Python::FloatToValue(bc.Size.y);
                    data[5] = Python::FloatToValue(bc.Size.z);
                    data[6] = Python::FloatToValue(bc.Offset.x);
                    data[7] = Python::FloatToValue(bc.Offset.y);
                    data[8] = Python::FloatToValue(bc.Offset.z);
                    elements[arrayIndex++] = Python::MakeTuple(data, 9);
                }
                else if (entity.HasComponent<SphereColliderComponent>())
                {
                    auto& sc = entity.GetComponent<SphereColliderComponent>();
                    Python::ScriptRef data[4];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(1); // Sphere
                    data[2] = Python::BoolToValue(sc.IsTrigger);
                    data[3] = Python::FloatToValue(sc.Radius);
                    elements[arrayIndex++] = Python::MakeTuple(data, 4);
                }
                else if (entity.HasComponent<CapsuleColliderComponent>())
                {
                    auto& cc = entity.GetComponent<CapsuleColliderComponent>();
                    Python::ScriptRef data[5];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(2); // Capsule
                    data[2] = Python::BoolToValue(cc.IsTrigger);
                    data[3] = Python::FloatToValue(cc.Radius);
                    data[4] = Python::FloatToValue(cc.Height);
                    elements[arrayIndex++] = Python::MakeTuple(data, 5);
                }
                else if (entity.HasComponent<MeshColliderComponent>())
                {
                    auto& mc = entity.GetComponent<MeshColliderComponent>();
                    Python::ScriptRef data[4];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(3); // Mesh
                    data[2] = Python::BoolToValue(mc.IsTrigger);
                    data[3] = Python::UInt64ToValue((uint64_t)new Ref<Mesh>(mc.CollisionMesh));
                    elements[arrayIndex++] = Python::MakeTuple(data, 4);
                }
            }

            Python::ScriptRef tuple = Python::MakeTuple(elements, arrayIndex);
            delete[] elements;
            return tuple.Detach();
        }

        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Physics_OverlapCapsule(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        glm::vec3 origin = Python::ValueToVec3(Python::GetTupleElement(argsRef, 0));
        float radius = Python::ValueToFloat(Python::GetTupleElement(argsRef, 1));
        float halfHeight = Python::ValueToFloat(Python::GetTupleElement(argsRef, 2));

        std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
        uint32_t count;
        if (PXPhysicsWrappers::OverlapCapsule(origin, radius, halfHeight, buffer, &count))
        {
            Python::ScriptRef* elements = new Python::ScriptRef[count];
            uint32_t arrayIndex = 0;

            for (uint32_t i = 0; i < count; i++)
            {
                Entity& entity = *(Entity*)buffer[i].actor->userData;

                if (entity.HasComponent<BoxColliderComponent>())
                {
                    auto& bc = entity.GetComponent<BoxColliderComponent>();
                    Python::ScriptRef data[9];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(0);
                    data[2] = Python::BoolToValue(bc.IsTrigger);
                    data[3] = Python::FloatToValue(bc.Size.x);
                    data[4] = Python::FloatToValue(bc.Size.y);
                    data[5] = Python::FloatToValue(bc.Size.z);
                    data[6] = Python::FloatToValue(bc.Offset.x);
                    data[7] = Python::FloatToValue(bc.Offset.y);
                    data[8] = Python::FloatToValue(bc.Offset.z);
                    elements[arrayIndex++] = Python::MakeTuple(data, 9);
                }
                else if (entity.HasComponent<SphereColliderComponent>())
                {
                    auto& sc = entity.GetComponent<SphereColliderComponent>();
                    Python::ScriptRef data[4];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(1);
                    data[2] = Python::BoolToValue(sc.IsTrigger);
                    data[3] = Python::FloatToValue(sc.Radius);
                    elements[arrayIndex++] = Python::MakeTuple(data, 4);
                }
                else if (entity.HasComponent<CapsuleColliderComponent>())
                {
                    auto& cc = entity.GetComponent<CapsuleColliderComponent>();
                    Python::ScriptRef data[5];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(2);
                    data[2] = Python::BoolToValue(cc.IsTrigger);
                    data[3] = Python::FloatToValue(cc.Radius);
                    data[4] = Python::FloatToValue(cc.Height);
                    elements[arrayIndex++] = Python::MakeTuple(data, 5);
                }
                else if (entity.HasComponent<MeshColliderComponent>())
                {
                    auto& mc = entity.GetComponent<MeshColliderComponent>();
                    Python::ScriptRef data[4];
                    data[0] = Python::UInt64ToValue(entity.GetUUID());
                    data[1] = Python::UInt64ToValue(3);
                    data[2] = Python::BoolToValue(mc.IsTrigger);
                    data[3] = Python::UInt64ToValue((uint64_t)new Ref<Mesh>(mc.CollisionMesh));
                    elements[arrayIndex++] = Python::MakeTuple(data, 4);
                }
            }

            Python::ScriptRef tuple = Python::MakeTuple(elements, arrayIndex);
            delete[] elements;
            return tuple.Detach();
        }

        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Physics_GetGravity(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        return Python::FloatToValue(Physics::GetGravity()).Detach();
    }

    Python::ScriptValue* Prism_Physics_SetGravity(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        float gravity = Python::ValueToFloat(Python::GetTupleElement(argsRef, 0));
        Physics::SetGravity(gravity);
        return Python::NoneValue().Detach();
    }
#pragma endregion

#pragma region Mesh

    Python::ScriptValue* Prism_Mesh_Constructor(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        std::string filepath = Python::ValueToString(Python::GetTupleElement(argsRef, 0));
        auto* ref = new Ref<Mesh>(ModelImporter::Import(filepath).Mesh);
        return Python::UInt64ToValue(reinterpret_cast<uint64_t>(ref)).Detach();
    }

    Python::ScriptValue* Prism_Mesh_Destructor(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        delete reinterpret_cast<Ref<Mesh>*>(handle);
        return Python::NoneValue().Detach();
    }


#pragma region MeshFactory

    Python::ScriptValue* Prism_MeshFactory_CreatePlane(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        float width = Python::ValueToFloat(Python::GetTupleElement(argsRef, 0));
        float height = Python::ValueToFloat(Python::GetTupleElement(argsRef, 1));
        auto* ref = new Ref<Mesh>(ModelImporter::Import("assets/models/Plane1m.obj").Mesh);
        return Python::UInt64ToValue(reinterpret_cast<uint64_t>(ref)).Detach();
    }

#pragma endregion

#pragma region MeshRendererComponent

    Python::ScriptValue* Prism_MeshRendererComponent_GetMesh(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        auto& mc = entity.GetComponent<MeshRendererComponent>();
        if (mc.Mesh)
        {
            auto* ref = new Ref<Mesh>(mc.Mesh);
            return Python::UInt64ToValue(reinterpret_cast<uint64_t>(ref)).Detach();
        }
        return Python::UInt64ToValue(0).Detach();
    }

    Python::ScriptValue* Prism_MeshRendererComponent_SetMesh(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 1));
        auto& mc = entity.GetComponent<MeshRendererComponent>();
        if (handle != 0)
        {
            auto& meshRef = *reinterpret_cast<Ref<Mesh>*>(handle);
            mc.Mesh = meshRef;
        }
        else
            mc.Mesh = nullptr;
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_MeshRendererComponent_GetMaterial(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        uint64_t index = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 1));
        auto& mc = entity.GetComponent<MeshRendererComponent>();
        if (index >= mc.Materials.size())
            return Python::UInt64ToValue(0).Detach();
        if (mc.Materials[index])
        {
            auto* ref = new Ref<Material>(mc.Materials[index]);
            return Python::UInt64ToValue(reinterpret_cast<uint64_t>(ref)).Detach();
        }
        return Python::UInt64ToValue(0).Detach();
    }

    Python::ScriptValue* Prism_MeshRendererComponent_SetMaterial(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        uint64_t index = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 1));
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 2));
        auto& mc = entity.GetComponent<MeshRendererComponent>();
        if (index >= mc.Materials.size())
            return Python::NoneValue().Detach();
        if (handle != 0)
        {
            auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
            mc.Materials[index] = matRef;
        }
        else
            mc.Materials[index] = nullptr;
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_MeshRendererComponent_GetMaterialCount(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        auto& mc = entity.GetComponent<MeshRendererComponent>();
        return Python::UInt64ToValue(mc.Materials.size()).Detach();
    }

    Python::ScriptValue* Prism_MeshRendererComponent_GetMaterials(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        auto& mc = entity.GetComponent<MeshRendererComponent>();
        size_t count = mc.Materials.size();
        Python::ScriptRef* elements = new Python::ScriptRef[count];
        for (size_t i = 0; i < count; i++)
        {
            if (mc.Materials[i])
            {
                auto* ref = new Ref<Material>(mc.Materials[i]);
                elements[i] = Python::UInt64ToValue(reinterpret_cast<uint64_t>(ref));
            }
            else
                elements[i] = Python::UInt64ToValue(0);
        }
        Python::ScriptRef tuple = Python::MakeTuple(elements, (uint32_t)count);
        delete[] elements;
        return tuple.Detach();
    }

    Python::ScriptValue* Prism_MeshRendererComponent_SetMaterials(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef handlesTuple = Python::GetTupleElement(argsRef, 1);
        size_t count = Python::GetTupleSize(handlesTuple);
        auto& mc = entity.GetComponent<MeshRendererComponent>();
        mc.Materials.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(handlesTuple, (uint32_t)i));
            if (handle != 0)
            {
                auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
                mc.Materials[i] = matRef;
            }
            else
                mc.Materials[i] = nullptr;
        }
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region Texture2D

    Python::ScriptValue* Prism_Texture2D_Constructor(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint32_t width = (uint32_t)Python::ValueToInt(Python::GetTupleElement(argsRef, 0));
        uint32_t height = (uint32_t)Python::ValueToInt(Python::GetTupleElement(argsRef, 1));
        auto tex = Texture2D::Create(TextureFormat::RGBA, width, height);
        auto* ref = new Ref<Texture2D>(tex);
        return Python::UInt64ToValue(reinterpret_cast<uint64_t>(ref)).Detach();
    }

    Python::ScriptValue* Prism_Texture2D_Destructor(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        delete reinterpret_cast<Ref<Texture2D>*>(handle);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Texture2D_SetData(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        auto& texRef = *reinterpret_cast<Ref<Texture2D>*>(handle);

        Python::ScriptRef dataTuple = Python::GetTupleElement(argsRef, 1);
        size_t count = Python::GetTupleSize(dataTuple);

        texRef->Lock();
        Buffer buffer = texRef->GetWriteableBuffer();
        uint8_t* pixels = static_cast<uint8_t*>(buffer.Data);
        uint32_t pixelCount = texRef->GetWidth() * texRef->GetHeight();

        for (size_t i = 0; i < count && i < pixelCount; i++)
        {
            auto elem = Python::GetTupleElement(dataTuple, (uint32_t)i);
            glm::vec4 color = Python::ValueToVec4(elem);
            *pixels++ = static_cast<uint8_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
            *pixels++ = static_cast<uint8_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
            *pixels++ = static_cast<uint8_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);
            *pixels++ = static_cast<uint8_t>(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f);
        }

        texRef->Unlock();
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region Material

    Python::ScriptValue* Prism_Material_Constructor(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        std::string shaderName = Python::ValueToString(Python::GetTupleElement(argsRef, 0));
        const auto& shader = Renderer::GetShaderLibrary()->Get(shaderName);
        auto* ref = new Ref<Material>(Material::Create(shader));
        return Python::UInt64ToValue(reinterpret_cast<uint64_t>(ref)).Detach();
    }

    Python::ScriptValue* Prism_Material_Destructor(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        delete reinterpret_cast<Ref<Material>*>(handle);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetFloat(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        float value = Python::ValueToFloat(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetFloat(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetInt(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        int value = Python::ValueToInt(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetInt(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetBool(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        bool value = Python::ValueToBool(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetBool(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetVector2(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        glm::vec2 value = Python::ValueToVec2(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetVec2(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetColor3(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        glm::vec3 value = Python::ValueToVec3(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetColor3(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetColor(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        glm::vec4 value = Python::ValueToVec4(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetColor(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetMatrix4(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        glm::mat4 value = Python::ValueToMat4(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetMatrix4(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetVector3(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        glm::vec3 value = Python::ValueToVec3(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetVec3(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetVector4(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        glm::vec4 value = Python::ValueToVec4(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetVec4(uniform, value);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetTexture(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string uniform = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        uint64_t texHandle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        auto& texRef = *reinterpret_cast<Ref<Texture2D>*>(texHandle);
        matRef->SetTexture(uniform, texRef);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_SetKeyword(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string keyword = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        bool enabled = Python::ValueToBool(Python::GetTupleElement(argsRef, 2));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        matRef->SetKeyword(keyword, enabled);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_Material_IsKeywordEnabled(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        uint64_t handle = Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0));
        std::string keyword = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        auto& matRef = *reinterpret_cast<Ref<Material>*>(handle);
        return Python::BoolToValue(matRef->IsKeywordEnabled(keyword)).Detach();
    }

#pragma endregion


#pragma endregion


} // namespace Prism::Script
