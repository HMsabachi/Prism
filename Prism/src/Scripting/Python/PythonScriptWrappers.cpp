#include "prpch.h"
#include "PythonScriptWrappers.h"

#include "Prism/Core/Input.h"
#include "Prism/Core/KeyCodes.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"

#include "Scripting/Python/PythonScriptEngine.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"
#include "Scripting/Python/Interop/PythonMathBridge.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <box2d/box2d.h>
#include <PhysX/PxPhysicsAPI.h>

namespace Prism {
    extern std::unordered_map<uint64_t, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
    extern std::unordered_map<uint64_t, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;
}

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
        message = "[Python]: " + message;

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

#pragma endregion

#pragma region Input

    Python::ScriptValue* Prism_Input_IsKeyPressed(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        KeyCode key = static_cast<KeyCode>(Python::ValueToInt(Python::GetTupleElement(argsRef, 0)));
        return Python::BoolToValue(Input::IsKeyPressed(key)).Detach();
    }

#pragma endregion

#pragma region Entity

    Python::ScriptValue* Prism_Entity_GetTransform(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        auto& tc = entity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        return Python::Mat4ToValue(transform).Detach();
    }

    Python::ScriptValue* Prism_Entity_SetTransform(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef matObj = Python::GetTupleElement(argsRef, 1);

        glm::mat4 transform = Python::ValueToMat4(matObj);
        auto& tc = entity.GetComponent<TransformComponent>();
        tc.SetTransform(transform);
        return Python::NoneValue().Detach();
    }

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
        std::string moduleName = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
        std::string className = Python::ValueToString(Python::GetTupleElement(argsRef, 2));

        UUID sceneID = PythonScriptEngine::GetCurrentSceneContext()->GetUUID();
        UUID behaviourID = UUID();

        Python::ScriptModule mod = Python::ScriptModule::Import(moduleName.c_str());
        PR_CORE_ASSERT(mod.IsValid(), "Python module not found!");
        Python::ScriptClass cls = Python::ScriptClass::From(mod, className.c_str());
        PR_CORE_ASSERT(cls.IsValid(), "Python class not found!");
        Python::ScriptObject obj = cls.CreateInstance();
        PR_CORE_ASSERT(obj.IsValid(), "Failed to create instance!");

        UUID entityUUID = entity.GetUUID();
        Python::ScriptObject* entityObj = PythonScriptEngine::GetScriptObject(sceneID, entityUUID);
        if (entityObj)
            obj.SetAttribute("Entity", entityObj->GetRef());

        auto& sceneMap = PythonScriptEngine::s_PythonScriptObjects[sceneID];
        auto [it, inserted] = sceneMap.emplace(behaviourID, std::move(obj));
        PR_CORE_ASSERT(inserted, "BehaviourID collision!");

        auto& comp = entity.GetComponent<PythonScriptComponent>();
        auto& binding = comp.Behaviours.emplace_back();
        binding.BehaviourID = behaviourID;
        binding.ModuleName = moduleName;
        binding.ClassName = className;
        if (auto* meta = PythonScriptMetaRegistry::GetClassMetadata(moduleName + "." + className))
            binding.LifecycleMask = meta->LifecycleMask;

        it->second.Invoke<void>("Awake");
        it->second.Invoke<void>("OnCreate");
        if (it->second.GetField<bool>("enabled"))
            it->second.Invoke<void>("OnEnable");

        return it->second.GetRef().Detach();
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
        PythonScriptEngine::RemoveBehaviour(entity, behaviourID);
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_GetRotation(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        auto& tc = entity.GetComponent<TransformComponent>();
        glm::vec3 euler = glm::eulerAngles(tc.Rotation);

        return Python::Vec3ToValue(euler).Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_SetRotation(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);

        glm::vec3 euler = Python::ValueToVec3(vecObj);
        auto& tc = entity.GetComponent<TransformComponent>();
        tc.Rotation = glm::quat(glm::radians(euler));
        return Python::NoneValue().Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_GetScale(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        auto& tc = entity.GetComponent<TransformComponent>();

        return Python::Vec3ToValue(tc.Scale).Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_SetScale(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);

        auto& tc = entity.GetComponent<TransformComponent>();
        tc.Scale = Python::ValueToVec3(vecObj);
        return Python::NoneValue().Detach();
    }

#pragma endregion

#pragma region TransformComponent

    Python::ScriptValue* Prism_TransformComponent_GetPosition(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        auto& tc = entity.GetComponent<TransformComponent>();

        return Python::Vec3ToValue(tc.Position).Detach();
    }

    Python::ScriptValue* Prism_TransformComponent_SetPosition(Python::ScriptValue* self, Python::ScriptValue* args)
    {
        Python::ScriptRef argsRef(args);
        Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
        Python::ScriptRef vecObj = Python::GetTupleElement(argsRef, 1);

        auto& tc = entity.GetComponent<TransformComponent>();
        tc.Position = Python::ValueToVec3(vecObj);
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
        dynamicActor->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
        return Python::NoneValue().Detach();
    }

#pragma endregion

    void RegisterPrismModule()
    {
        Python::NativeModule mod("PrismNative");

#define PR_PYTHON_FUNCTION(func, doc) mod.AddFunction(#func, func, doc)
        // Log
        PR_PYTHON_FUNCTION(Prism_Log_LogMessage, "Log(level, message)");

        // Time
        PR_PYTHON_FUNCTION(Prism_Time_GetDeltaTime, "GetDeltaTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetTime, "GetTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetUnscaledDeltaTime, "GetUnscaledDeltaTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetUnscaledTime, "GetUnscaledTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetFixedDeltaTime, "GetFixedDeltaTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetFrameCount, "GetFrameCount() -> uint64");
        PR_PYTHON_FUNCTION(Prism_Time_GetTimeScale, "GetTimeScale() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_SetTimeScale, "SetTimeScale(scale)");

        // Input
        PR_PYTHON_FUNCTION(Prism_Input_IsKeyPressed, "IsKeyPressed(key) -> bool");

        // Entity
        PR_PYTHON_FUNCTION(Prism_Entity_GetTransform, "GetTransform(entityID) -> mat4");
        PR_PYTHON_FUNCTION(Prism_Entity_SetTransform, "SetTransform(entityID, mat4)");
        PR_PYTHON_FUNCTION(Prism_Entity_CreateComponent, "CreateComponent(entityID, typeName)");
        PR_PYTHON_FUNCTION(Prism_Entity_HasComponent, "HasComponent(entityID, typeName) -> bool");
        PR_PYTHON_FUNCTION(Prism_Entity_AddBehaviour, "AddBehaviour(entityID, moduleName, className) -> object");
        PR_PYTHON_FUNCTION(Prism_Entity_FindEntityByTag, "FindEntityByTag(tag) -> uint64");
        PR_PYTHON_FUNCTION(Prism_Entity_RemoveBehaviour, "RemoveBehaviour(entityID, behaviourID)");

        // TagComponent
        PR_PYTHON_FUNCTION(Prism_TagComponent_GetTag, "GetTag(entityID) -> string");
        PR_PYTHON_FUNCTION(Prism_TagComponent_SetTag, "SetTag(entityID, tag)");

        // TransformComponent
        PR_PYTHON_FUNCTION(Prism_TransformComponent_GetPosition, "GetPosition(entityID) -> vec3");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_SetPosition, "SetPosition(entityID, vec3)");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_GetRotation, "GetRotation(entityID) -> vec3 radians");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_SetRotation, "SetRotation(entityID, vec3) degrees");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_GetScale, "GetScale(entityID) -> vec3");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_SetScale, "SetScale(entityID, vec3)");

        // RigidBody2DComponent
        PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_ApplyLinearImpulse, "ApplyLinearImpulse(entityID, impulse, offset, wake)");
        PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_GetLinearVelocity, "GetLinearVelocity(entityID) -> vec2");
        PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_SetLinearVelocity, "SetLinearVelocity(entityID, velocity)");

        // RigidBodyComponent (3D)
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_AddForce, "AddForce(entityID, force, forceMode)");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_AddTorque, "AddTorque(entityID, torque, forceMode)");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_GetLinearVelocity, "GetLinearVelocity(entityID) -> vec3");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_SetLinearVelocity, "SetLinearVelocity(entityID, velocity)");

        mod.Register();

        PR_CORE_TRACE("[Python] PrismNative 模块已注册");
    }

} // namespace Prism::Script
