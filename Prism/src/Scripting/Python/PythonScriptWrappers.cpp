#include "prpch.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Core/Math/Noise.h"
#include "Prism/Core/Input.h"
#include "Prism/Physics/PXPhysicsWrappers.h"
#include "Prism/Physics/Physics.h"
#include "Prism/Physics/PhysicsActor.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include "Prism/Scene/Systems/ScriptSystem.h"
#include "Prism/Scene/Systems/TransformSystem.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Asset/ModelImporter.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <glm/gtc/type_ptr.hpp>
#include <box2d/box2d.h>
#include <PhysX/PxPhysicsAPI.h>
#include "Prism/Physics/PhysicsUtil.h"
#include "PythonScriptWrappers.h"
#include "PythonScriptTypeCasters.h"

namespace py = pybind11;
using namespace Prism;

namespace Prism
{
    extern std::unordered_map<uint64_t, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
    extern std::unordered_map<uint64_t, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;
}


//  Helper

namespace Prism
{
    namespace PythonScript
    {

        static Entity GetEntityFromEntityID(uint64_t entityID)
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            const auto& entityMap = scene->GetEntityMap();
            PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(),
                "Invalid entity ID or entity doesn't exist in scene!");
            return entityMap.at(entityID);
        }

#pragma region Log

        void Prism_Log_LogMessage(int32_t level, const char* message)
        {
            std::string msg = "[Python] ";
            msg += message;
            auto lvl = static_cast<LogLevel>(level);
            switch (lvl)
            {
            case LogLevel::Trace:
                PR_CORE_TRACE(msg);
                break;
            case LogLevel::Debug:
                PR_CORE_INFO(msg);
                break;
            case LogLevel::Info:
                PR_CORE_INFO(msg);
                break;
            case LogLevel::Warn:
                PR_CORE_WARN(msg);
                break;
            case LogLevel::Error:
                PR_CORE_ERROR(msg);
                break;
            case LogLevel::Critical:
                PR_CORE_FATAL(msg);
                break;
            }
        }

#pragma region Time

        float Prism_Time_GetDeltaTime() { return Time::GetDeltaTime(); }
        float Prism_Time_GetUnscaledDeltaTime() { return Time::GetUnscaledDeltaTime(); }
        float Prism_Time_GetTime() { return Time::GetTime(); }
        float Prism_Time_GetUnscaledTime() { return Time::GetUnscaledTime(); }
        float Prism_Time_GetFixedDeltaTime() { return Time::GetFixedDeltaTime(); }
        int64_t Prism_Time_GetFrameCount() { return (int64_t)Time::GetFrameCount(); }
        void Prism_Time_SetTimeScale(float scale) { Time::SetTimeScale(scale); }
        float Prism_Time_GetTimeScale() { return Time::GetTimeScale(); }
        void Prism_Time_SetFixedDeltaTime(float fixedDeltaTime) { Time::SetFixedDeltaTime(fixedDeltaTime); }

#pragma endregion
#pragma region Math

        float Prism_Noise_PerlinNoise(float x, float y) { return Noise::PerlinNoise(x, y); }

#pragma region Input

        bool Prism_Input_IsKeyPressed(uint16_t key) { return Input::IsKeyPressed((KeyCode)key); }
        glm::vec2 Prism_Input_GetMousePosition()
        {
            auto [x, y] = Input::GetMousePosition();
            return { x, y };
        }
        void Prism_Input_SetCursorMode(int mode) { Input::SetCursorMode((CursorMode)mode); }
        int Prism_Input_GetCursorMode() { return (int)Input::GetCursorMode(); }
        bool Prism_Input_IsMouseButtonPressed(uint16_t button) { return Input::IsMouseButtonPressed((MouseButton)button); }

        static TransformSystem* GetTransformSystem(Entity entity)
        {
            return entity.GetScene()->GetSystem<TransformSystem>();
        }

#pragma region MeshRendererComponent

        uint64_t Prism_MeshRendererComponent_GetMesh(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& mc = entity.GetComponent<MeshRendererComponent>();
            return reinterpret_cast<uintptr_t>(new Ref<Mesh>(mc.Mesh));
        }

        void Prism_MeshRendererComponent_SetMesh(uint64_t entityID, uint64_t meshHandle)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& mc = entity.GetComponent<MeshRendererComponent>();
            mc.Mesh = meshHandle ? *reinterpret_cast<Ref<Mesh> *>(meshHandle) : nullptr;
        }

        uint64_t Prism_MeshRendererComponent_GetMaterial(uint64_t entityID, uint64_t index)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& mc = entity.GetComponent<MeshRendererComponent>();
            PR_CORE_ASSERT(index < mc.Materials.size(), "Material index out of range");
            return reinterpret_cast<uintptr_t>(new Ref<Material>(mc.Materials[index]));
        }

        void Prism_MeshRendererComponent_SetMaterial(uint64_t entityID, uint64_t materialHandle, uint64_t index)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& mc = entity.GetComponent<MeshRendererComponent>();
            PR_CORE_ASSERT(index < mc.Materials.size(), "Material index out of range");
            mc.Materials[index] = *reinterpret_cast<Ref<Material> *>(materialHandle);
        }

        uint64_t Prism_MeshRendererComponent_GetMaterialCount(uint64_t entityID)
        {
            return GetEntityFromEntityID(entityID).GetComponent<MeshRendererComponent>().Materials.size();
        }

        std::vector<uint64_t> Prism_MeshRendererComponent_GetMaterials(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& mc = entity.GetComponent<MeshRendererComponent>();
            std::vector<uint64_t> result;
            result.reserve(mc.Materials.size());
            for (auto& mat : mc.Materials)
            {
                if (mat)
                    result.push_back(reinterpret_cast<uintptr_t>(new Ref<Material>(mat)));
                else
                    result.push_back(0);
            }
            return result;
        }

        void Prism_MeshRendererComponent_SetMaterials(uint64_t entityID, const std::vector<uint64_t>& handles)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& mc = entity.GetComponent<MeshRendererComponent>();
            mc.Materials.resize(handles.size());
            for (size_t i = 0; i < handles.size(); ++i)
            {
                if (handles[i])
                    mc.Materials[i] = *reinterpret_cast<Ref<Material> *>(handles[i]);
                else
                    mc.Materials[i] = nullptr;
            }
        }

#pragma region Mesh

        uint64_t Prism_Mesh_Constructor(const char* filepath)
        {
            auto result = ModelImporter::Import(filepath);
            return reinterpret_cast<uintptr_t>(new Ref<Mesh>(result.Mesh));
        }

        void Prism_Mesh_Destructor(uint64_t handle)
        {
            delete reinterpret_cast<Ref<Mesh>*>(handle);
        }

        uint64_t Prism_MeshFactory_CreatePlane(float width, float height)
        {
            return reinterpret_cast<uintptr_t>(
                new Ref<Mesh>(ModelImporter::Import("assets/models/Plane1m.obj").Mesh));
        }

#pragma region Texture2D

        uint64_t Prism_Texture2D_Constructor(uint32_t width, uint32_t height)
        {
            return reinterpret_cast<uintptr_t>(
                new Ref<Texture2D>(Texture2D::Create(TextureFormat::RGBA, width, height)));
        }

        void Prism_Texture2D_Destructor(uint64_t handle)
        {
            delete reinterpret_cast<Ref<Texture2D>*>(handle);
        }

#pragma region RigidBody2DComponent

        void Prism_RigidBody2DComponent_ApplyLinearImpulse(
            uint64_t entityID, const glm::vec2& impulse, const glm::vec2& offset, bool wake)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y),
                b2Vec2(offset.x, offset.y), wake);
        }

        glm::vec2 Prism_RigidBody2DComponent_GetLinearVelocity(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            const b2Vec2& v = body->GetLinearVelocity();
            return { v.x, v.y };
        }

        void Prism_RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, const glm::vec2& velocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
        }

#pragma region RigidBodyComponent

        void Prism_RigidBodyComponent_AddForce(uint64_t entityID, const glm::vec3& force, int32_t forceMode)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
            actor->AddForce(force, (ForceMode)forceMode);
        }
        void Prism_RigidBodyComponent_AddTorque(uint64_t entityID, const glm::vec3& torque, int32_t forceMode)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
            actor->AddTorque(torque, (ForceMode)forceMode);
        }
        glm::vec3 Prism_RigidBodyComponent_GetLinearVelocity(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetLinearVelocity();
        }
        void Prism_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, const glm::vec3& velocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetLinearVelocity(velocity);
        }
        void Prism_RigidBodyComponent_Rotate(uint64_t entityID, const glm::vec3& rotation)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->Rotate(rotation);
        }
        uint32_t Prism_RigidBodyComponent_GetLayer(uint64_t entityID)
        {
            return GetEntityFromEntityID(entityID).GetComponent<RigidBodyComponent>().Layer;
        }
        float Prism_RigidBodyComponent_GetMass(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetMass();
        }
        void Prism_RigidBodyComponent_SetMass(uint64_t entityID, float mass)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetMass(mass);
        }
        uint32_t Prism_RigidBodyComponent_GetBodyType(uint64_t entityID)
        {
            return (uint32_t)GetEntityFromEntityID(entityID).GetComponent<RigidBodyComponent>().BodyType;
        }
        glm::vec3 Prism_RigidBodyComponent_GetAngularVelocity(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetAngularVelocity();
        }
        void Prism_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, const glm::vec3& velocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetAngularVelocity(velocity);
        }

#pragma region Physics

        // Shared helper — populates OverlapHitData from PxOverlapHit
        static OverlapHitData FillOverlapHit(physx::PxOverlapHit& pxHit)
        {
            Entity& entity = *(Entity*)pxHit.actor->userData;
            OverlapHitData data{};
            data.EntityID = entity.GetUUID();

            if (entity.HasComponent<BoxColliderComponent>())
            {
                auto& bc = entity.GetComponent<BoxColliderComponent>();
                data.ColliderType = 0;
                data.IsTrigger = bc.IsTrigger;
                data.ShapeData[0] = bc.Size.x;
                data.ShapeData[1] = bc.Size.y;
                data.ShapeData[2] = bc.Size.z;
                data.ShapeData[3] = bc.Offset.x;
                data.ShapeData[4] = bc.Offset.y;
                data.ShapeData[5] = bc.Offset.z;
                data.MeshHandle = nullptr;
            }
            else if (entity.HasComponent<SphereColliderComponent>())
            {
                auto& sc = entity.GetComponent<SphereColliderComponent>();
                data.ColliderType = 1;
                data.IsTrigger = sc.IsTrigger;
                data.ShapeData[0] = sc.Radius;
                data.MeshHandle = nullptr;
            }
            else if (entity.HasComponent<CapsuleColliderComponent>())
            {
                auto& cc = entity.GetComponent<CapsuleColliderComponent>();
                data.ColliderType = 2;
                data.IsTrigger = cc.IsTrigger;
                data.ShapeData[0] = cc.Radius;
                data.ShapeData[1] = cc.Height;
                data.MeshHandle = nullptr;
            }
            else if (entity.HasComponent<MeshColliderComponent>())
            {
                auto& mc = entity.GetComponent<MeshColliderComponent>();
                data.ColliderType = 3;
                data.IsTrigger = mc.IsTrigger;
                data.MeshHandle = new Ref<Mesh>(mc.CollisionMesh);
            }
            return data;
        }

        std::optional<RaycastHit> Prism_Physics_Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance)
        {
            RaycastHit hit;
            if (PXPhysicsWrappers::Raycast(origin, direction, maxDistance, &hit))
                return hit;
            return std::nullopt;
        }

        std::vector<OverlapHitData> Prism_Physics_OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            std::vector<OverlapHitData> results;
            if (PXPhysicsWrappers::OverlapBox(origin, halfSize, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }

        std::vector<OverlapHitData> Prism_Physics_OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            std::vector<OverlapHitData> results;
            if (PXPhysicsWrappers::OverlapCapsule(origin, radius, halfHeight, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }

        std::vector<OverlapHitData> Prism_Physics_OverlapSphere(const glm::vec3& origin, float radius)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            std::vector<OverlapHitData> results;
            if (PXPhysicsWrappers::OverlapSphere(origin, radius, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }

        float Prism_Physics_GetGravity() { return Physics::GetGravity(); }
        void Prism_Physics_SetGravity(float gravity) { Physics::SetGravity(gravity); }

        // Material
        static Ref<Material>& DerefMaterial(uint64_t handle)
        {
            return *reinterpret_cast<Ref<Material> *>(handle);
        }

        uint64_t Prism_Material_Constructor(const char* shaderName)
        {
            const auto& shader = AssetManager::GetShaderLibrary()->Get(shaderName);
            return reinterpret_cast<uintptr_t>(new Ref<Material>(Material::Create(shader->Handle)));
        }

        void Prism_Material_Destructor(uint64_t handle)
        {
            delete reinterpret_cast<Ref<Material>*>(handle);
        }

        void Prism_Material_SetFloat(uint64_t handle, const char* uniform, float value)
        {
            DerefMaterial(handle)->SetFloat(uniform, value);
        }
        void Prism_Material_SetInt(uint64_t handle, const char* uniform, int value)
        {
            DerefMaterial(handle)->SetInt(uniform, value);
        }
        void Prism_Material_SetBool(uint64_t handle, const char* uniform, bool value)
        {
            DerefMaterial(handle)->SetBool(uniform, value);
        }
        void Prism_Material_SetVector2(uint64_t handle, const char* uniform, const glm::vec2& value)
        {
            DerefMaterial(handle)->SetVec2(uniform, value);
        }
        void Prism_Material_SetVector3(uint64_t handle, const char* uniform, const glm::vec3& value)
        {
            DerefMaterial(handle)->SetVec3(uniform, value);
        }
        void Prism_Material_SetVector4(uint64_t handle, const char* uniform, const glm::vec4& value)
        {
            DerefMaterial(handle)->SetVec4(uniform, value);
        }
        void Prism_Material_SetColor3(uint64_t handle, const char* uniform, const glm::vec3& value)
        {
            DerefMaterial(handle)->SetColor3(uniform, value);
        }
        void Prism_Material_SetColor(uint64_t handle, const char* uniform, const glm::vec4& value)
        {
            DerefMaterial(handle)->SetColor(uniform, value);
        }
        void Prism_Material_SetMatrix4(uint64_t handle, const char* uniform, const glm::mat4& value)
        {
            DerefMaterial(handle)->SetMatrix4(uniform, value);
        }
        void Prism_Material_SetTexture(uint64_t handle, const char* uniform, uint64_t textureHandle)
        {
            DerefMaterial(handle)->SetTexture(uniform, *reinterpret_cast<Ref<Texture2D> *>(textureHandle));
        }
        void Prism_Material_SetKeyword(uint64_t handle, const char* name, bool enabled)
        {
            DerefMaterial(handle)->SetKeyword(name, enabled);
        }
        bool Prism_Material_IsKeywordEnabled(uint64_t handle, const char* name)
        {
            return DerefMaterial(handle)->IsKeywordEnabled(name);
        }

    }
} // namespace Prism::PythonScript

// PriseNative Register
PYBIND11_MODULE(PrismNative, m)
{
    using namespace Prism;
    using namespace Prism::PythonScript;

    py::class_<RaycastHit>(m, "RaycastHit")
        .def_readwrite("EntityID", &RaycastHit::EntityID)
        .def_readwrite("Position", &RaycastHit::Position)
        .def_readwrite("Normal", &RaycastHit::Normal)
        .def_readwrite("Distance", &RaycastHit::Distance);

    py::class_<OverlapHitData>(m, "OverlapHitData")
        .def_readwrite("EntityID", &OverlapHitData::EntityID)
        .def_readwrite("ColliderType", &OverlapHitData::ColliderType)
        .def_readwrite("IsTrigger", &OverlapHitData::IsTrigger)
        .def_property_readonly("ShapeData", [](const OverlapHitData& d)
            {
                py::list shapeData;
                for (int i = 0; i < 6; ++i)
                    shapeData.append(PyFloat_FromDouble(d.ShapeData[i]));
                return shapeData; })
        .def_property_readonly("MeshHandle", [](const OverlapHitData& d) -> uint64_t
            { return reinterpret_cast<uintptr_t>(d.MeshHandle); });

    py::class_<ScriptTransform>(m, "ScriptTransform")
        .def(py::init<>())
        .def_readwrite("Position", &ScriptTransform::Position)
        .def_readwrite("Rotation", &ScriptTransform::Rotation)
        .def_readwrite("Scale", &ScriptTransform::Scale)
        .def_readwrite("Up", &ScriptTransform::Up)
        .def_readwrite("Right", &ScriptTransform::Right)
        .def_readwrite("Forward", &ScriptTransform::Forward);

#define BIND_MODULE_FUNCTION(name) m.def(#name, &Prism::PythonScript::name)
    // Log
    BIND_MODULE_FUNCTION(Prism_Log_LogMessage);
    // Time
    BIND_MODULE_FUNCTION(Prism_Time_GetDeltaTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetUnscaledDeltaTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetUnscaledTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetFixedDeltaTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetFrameCount);
    BIND_MODULE_FUNCTION(Prism_Time_SetTimeScale);
    BIND_MODULE_FUNCTION(Prism_Time_GetTimeScale);
    BIND_MODULE_FUNCTION(Prism_Time_SetFixedDeltaTime);
    // Math
    BIND_MODULE_FUNCTION(Prism_Noise_PerlinNoise);
    // Input
    BIND_MODULE_FUNCTION(Prism_Input_IsKeyPressed);
    BIND_MODULE_FUNCTION(Prism_Input_GetMousePosition);
    BIND_MODULE_FUNCTION(Prism_Input_SetCursorMode);
    BIND_MODULE_FUNCTION(Prism_Input_GetCursorMode);
    BIND_MODULE_FUNCTION(Prism_Input_IsMouseButtonPressed);
    // MeshRendererComponent
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_GetMesh);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_SetMesh);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_GetMaterial);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_SetMaterial);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_GetMaterialCount);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_GetMaterials);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_SetMaterials);
    // Mesh
    BIND_MODULE_FUNCTION(Prism_Mesh_Constructor);
    BIND_MODULE_FUNCTION(Prism_Mesh_Destructor);
    BIND_MODULE_FUNCTION(Prism_MeshFactory_CreatePlane);
    // Texture2D
    BIND_MODULE_FUNCTION(Prism_Texture2D_Constructor);
    BIND_MODULE_FUNCTION(Prism_Texture2D_Destructor);
    // RigidBody2DComponent
    BIND_MODULE_FUNCTION(Prism_RigidBody2DComponent_ApplyLinearImpulse);
    BIND_MODULE_FUNCTION(Prism_RigidBody2DComponent_GetLinearVelocity);
    BIND_MODULE_FUNCTION(Prism_RigidBody2DComponent_SetLinearVelocity);
    // RigidBodyComponent
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_AddForce);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_AddTorque);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetLinearVelocity);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_SetLinearVelocity);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_Rotate);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetLayer);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetMass);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_SetMass);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetBodyType);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetAngularVelocity);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_SetAngularVelocity);
    // Physics
    BIND_MODULE_FUNCTION(Prism_Physics_Raycast);
    BIND_MODULE_FUNCTION(Prism_Physics_OverlapBox);
    BIND_MODULE_FUNCTION(Prism_Physics_OverlapCapsule);
    BIND_MODULE_FUNCTION(Prism_Physics_OverlapSphere);
    BIND_MODULE_FUNCTION(Prism_Physics_GetGravity);
    BIND_MODULE_FUNCTION(Prism_Physics_SetGravity);
    // Material
    BIND_MODULE_FUNCTION(Prism_Material_Constructor);
    BIND_MODULE_FUNCTION(Prism_Material_Destructor);
    BIND_MODULE_FUNCTION(Prism_Material_SetFloat);
    BIND_MODULE_FUNCTION(Prism_Material_SetInt);
    BIND_MODULE_FUNCTION(Prism_Material_SetBool);
    BIND_MODULE_FUNCTION(Prism_Material_SetVector2);
    BIND_MODULE_FUNCTION(Prism_Material_SetVector3);
    BIND_MODULE_FUNCTION(Prism_Material_SetVector4);
    BIND_MODULE_FUNCTION(Prism_Material_SetColor3);
    BIND_MODULE_FUNCTION(Prism_Material_SetColor);
    BIND_MODULE_FUNCTION(Prism_Material_SetMatrix4);
    BIND_MODULE_FUNCTION(Prism_Material_SetTexture);
    BIND_MODULE_FUNCTION(Prism_Material_SetKeyword);
    BIND_MODULE_FUNCTION(Prism_Material_IsKeywordEnabled);
}

namespace Prism::PythonScript
{

    class PythonEntity
    {
        uint64_t m_EntityID = 0;

    public:
        PythonEntity(uint64_t id = 0) : m_EntityID(id) {}
        ~PythonEntity() { PR_CORE_TRACE("[Python] Destroyed Entity {0}", m_EntityID); }
        std::string __Repr__() { return fmt::format(" <Entity ID = {}>", m_EntityID); }

        uint64_t GetID() const { return m_EntityID; }
        void SetID(uint64_t id)
        {
            m_EntityID = id;
            PR_CORE_TRACE("[Python] Created Entity {0}", id);
        }

        pybind11::object GetComponent(pybind11::object cls)
        {
            py::object behaviourClass = py::module::import("Prism").attr("Behaviour");

            if (PyObject_IsSubclass(cls.ptr(), behaviourClass.ptr()) && cls.ptr() != behaviourClass.ptr())
            {
                // Behaviour lookup
                Entity entity = GetEntityFromEntityID(m_EntityID);
                std::string className = cls.attr("__module__").cast<std::string>() + "." + cls.attr("__qualname__").cast<std::string>();
                UUID classID = PythonScriptMetaRegistry::GenerateClassID(className);
                auto& comp = entity.GetComponent<PythonScriptComponent>();
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    if (binding.ClassID == classID)
                    {
                        UUID sceneID = PythonScriptEngine::GetCurrentSceneContext()->GetUUID();
                        py::object* obj = PythonScriptEngine::GetScriptObject(sceneID, UUID(bid));
                        if (obj) return *obj;
                        break;
                    }
                }
                return py::none();
            }

            // Component lookup
            Entity entity = GetEntityFromEntityID(m_EntityID);
            uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
            if (s_PythonHasComponentFuncs.count(typeId) && s_PythonHasComponentFuncs.at(typeId)(entity))
            {
                py::object component = cls();
                component.attr("Entity") = py::cast(this);
                return component;
            }
            return py::none();
        }

        pybind11::object CreateComponent(pybind11::object cls)
        {
            py::object behaviourClass = py::module::import("Prism").attr("Behaviour");

            if (PyObject_IsSubclass(cls.ptr(), behaviourClass.ptr()) && cls.ptr() != behaviourClass.ptr())
            {
                Entity entity = GetEntityFromEntityID(m_EntityID);
                std::string className = cls.attr("__module__").cast<std::string>() + "." + cls.attr("__qualname__").cast<std::string>();
                UUID classID = PythonScriptMetaRegistry::GenerateClassID(className);
                auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
                uint64_t bid = (uint64_t)ss->AddPythonBehaviour(entity, classID);
                UUID sceneID = PythonScriptEngine::GetCurrentSceneContext()->GetUUID();
                py::object* obj = PythonScriptEngine::GetScriptObject(sceneID, UUID(bid));
                if (obj) return *obj;
                return py::none();
            }

            Entity entity = GetEntityFromEntityID(m_EntityID);
            uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
            s_PythonCreateComponentFuncs.at(typeId)(entity);
            py::object component = cls();
            component.attr("Entity") = py::cast(this);
            return component;
        }

        bool HasComponent(pybind11::object cls)
        {
            py::object behaviourClass = py::module::import("Prism").attr("Behaviour");

            if (PyObject_IsSubclass(cls.ptr(), behaviourClass.ptr()) && cls.ptr() != behaviourClass.ptr())
            {
                Entity entity = GetEntityFromEntityID(m_EntityID);
                std::string className = cls.attr("__module__").cast<std::string>() + "." + cls.attr("__qualname__").cast<std::string>();
                UUID classID = PythonScriptMetaRegistry::GenerateClassID(className);
                auto& comp = entity.GetComponent<PythonScriptComponent>();
                for (auto& [bid, binding] : comp.Behaviours)
                    if (binding.ClassID == classID) return true;
                return false;
            }

            Entity entity = GetEntityFromEntityID(m_EntityID);
            uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
            return s_PythonHasComponentFuncs.count(typeId) && s_PythonHasComponentFuncs.at(typeId)(entity);
        }

        pybind11::object GetTransform()
        {
            py::object transformCompClass = py::module::import("Prism").attr("Component").attr("TransformComponent");
            return GetComponent(transformCompClass);
        }

        static pybind11::object FindEntityByTag(const char* tag)
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            std::string tagStr(tag);
            const auto& entityMap = scene->GetEntityMap();
            for (const auto& [id, entity] : entityMap)
            {
                if (entity.HasComponent<TagComponent>() &&
                    entity.GetComponent<TagComponent>().Tag == tagStr)
                    return py::cast(PythonEntity(id));
            }
            return py::none();
        }

        static pybind11::object FindEntityByID(uint64_t id)
        {
            return py::cast(PythonEntity(id));
        }
    };

    class PythonComponent
    {
    protected:
        pybind11::object m_Entity;
        uint64_t m_EntityID = 0;

    public:
        pybind11::object GetEntity() const { return m_Entity; }

        void SetEntity(pybind11::object entity)
        {
            m_Entity = entity;
            m_EntityID = entity.attr("ID").cast<uint64_t>();
        }
    };

    class PythonTransformComponent : public PythonComponent
    {
    public:
        glm::vec3 GetPosition() {
            Entity e = GetEntityFromEntityID(m_EntityID);
            return GetTransformSystem(e)->GetWorldPosition(e);
        }
        void SetPosition(const glm::vec3& v) {
            Entity e = GetEntityFromEntityID(m_EntityID);
            GetTransformSystem(e)->SetWorldPosition(e, v);
        }
        glm::vec3 GetRotation() {
            Entity e = GetEntityFromEntityID(m_EntityID);
            return GetTransformSystem(e)->GetWorldRotation(e);
        }
        void SetRotation(const glm::vec3& v) {
            Entity e = GetEntityFromEntityID(m_EntityID);
            GetTransformSystem(e)->SetWorldRotation(e, v);
        }
        glm::vec3 GetScale() {
            Entity e = GetEntityFromEntityID(m_EntityID);
            return GetTransformSystem(e)->GetWorldScale(e);
        }
        void SetScale(const glm::vec3& v) {
            Entity e = GetEntityFromEntityID(m_EntityID);
            GetTransformSystem(e)->SetWorldScale(e, v);
        }

        glm::vec3 GetLocalPosition() { return GetEntityFromEntityID(m_EntityID).Transformation().GetPosition(); }
        void SetLocalPosition(const glm::vec3& v) { GetEntityFromEntityID(m_EntityID).Transformation().SetPosition(v); }
        glm::vec3 GetLocalRotation() { return GetEntityFromEntityID(m_EntityID).Transformation().GetRotation(); }
        void SetLocalRotation(const glm::vec3& v) { GetEntityFromEntityID(m_EntityID).Transformation().SetRotation(v); }
        glm::vec3 GetLocalScale() { return GetEntityFromEntityID(m_EntityID).Transformation().GetScale(); }
        void SetLocalScale(const glm::vec3& v) { GetEntityFromEntityID(m_EntityID).Transformation().SetScale(v); }

        glm::vec3 GetForward() { return GetEntityFromEntityID(m_EntityID).Transformation().Forward; }
        glm::vec3 GetRight() { return GetEntityFromEntityID(m_EntityID).Transformation().Right; }
        glm::vec3 GetUp() { return GetEntityFromEntityID(m_EntityID).Transformation().Up; }

        ScriptTransform GetTransform() {
            Entity e = GetEntityFromEntityID(m_EntityID);
            auto world = GetTransformSystem(e)->GetWorldDecomposed(e);
            auto& tc = e.Transformation();
            return { world.Position, world.Rotation, world.Scale, tc.Up, tc.Right, tc.Forward };
        }
        void SetTransform(const ScriptTransform& t) {
            Entity e = GetEntityFromEntityID(m_EntityID);
            auto* ts = GetTransformSystem(e);
            ts->SetWorldPosition(e, t.Position);
            ts->SetWorldRotation(e, t.Rotation);
            ts->SetWorldScale(e, t.Scale);
        }
    };

    class PythonBehaviour : public PythonComponent
    {
        uint64_t m_BehaviourID = 0;

    public:
        uint64_t GetID() const { return m_BehaviourID; }
        void SetID(uint64_t id) { m_BehaviourID = id; }

        bool GetEnabled() {
            auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            return ss->GetEnabled(UUID(m_BehaviourID));
        }
        void SetEnabled(bool enabled) {
            auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            ss->SetEnabled(UUID(m_BehaviourID), enabled);
        }

        pybind11::object GetTransform()
        {
            if (!m_Entity.is_none())
                return m_Entity.attr("Transform");
            return pybind11::none();
        }

        pybind11::object GetComponent(pybind11::object cls)
        {
            if (!m_Entity.is_none())
                return m_Entity.attr("GetComponent")(cls);
            return pybind11::none();
        }

        bool HasComponent(pybind11::object cls)
        {
            if (!m_Entity.is_none())
                return m_Entity.attr("HasComponent")(cls).cast<bool>();
            return false;
        }

        pybind11::object CreateComponent(pybind11::object cls)
        {
            if (!m_Entity.is_none())
                return m_Entity.attr("CreateComponent")(cls);
            return pybind11::none();
        }
    };

    class PythonAsset
    {
    protected:
        uint64_t m_Handle = 0;
    public :
        PythonAsset(uint64_t handle) : m_Handle(handle) {}
        virtual ~PythonAsset() { if (m_Handle) delete (Ref<Asset>*)(m_Handle); m_Handle = 0;}
        virtual std::string __Repr__() { return fmt::format(" <Asset Handle = {}>", m_Handle); }
    };

    class PythonMesh : public PythonAsset
    {
    public:
        PythonMesh(uint64_t handle) : PythonAsset(handle) {}
        PythonMesh(const char* filepath) : PythonAsset(0)
        {
            auto result = ModelImporter::Import(filepath);
            m_Handle = reinterpret_cast<uint64_t>(new Ref<Mesh>(result.Mesh));
        }
        virtual ~PythonMesh() override { if (m_Handle) delete (Ref<Mesh>*)(m_Handle); m_Handle = 0; }
        virtual std::string __Repr__() override { return fmt::format(" <Mesh Handle = {}>", m_Handle); }
        uint64_t GetHandle() const { return m_Handle; } // TODO: Remove this
    };

} // namespace Prism::PythonScript

// PrismEngine Module Registe

PYBIND11_MODULE(PrismEngine, m)
{
    using namespace Prism::PythonScript;

    py::class_<PythonEntity>(m, "Entity")
        .def(py::init<uint64_t>(), py::arg("id") = 0)
        .def("__repr__", &PythonEntity::__Repr__)
        .def_property("ID", &PythonEntity::GetID, &PythonEntity::SetID)
        .def_property_readonly("_id", &PythonEntity::GetID)
        .def("GetComponent", &PythonEntity::GetComponent)
        .def("CreateComponent", &PythonEntity::CreateComponent)
        .def("HasComponent", &PythonEntity::HasComponent)
        .def_property_readonly("Transform", &PythonEntity::GetTransform)
        .def_static("FindEntityByTag", &PythonEntity::FindEntityByTag)
        .def_static("FindEntityByID", &PythonEntity::FindEntityByID);

    py::class_<PythonComponent>(m, "Component")
        .def(py::init<>())
        .def_property("Entity", &PythonComponent::GetEntity, &PythonComponent::SetEntity);

    py::class_<PythonTransformComponent, PythonComponent>(m, "TransformComponent")
        .def(py::init<>())
        .def_property("Position", &PythonTransformComponent::GetPosition, &PythonTransformComponent::SetPosition)
        .def_property("Rotation", &PythonTransformComponent::GetRotation, &PythonTransformComponent::SetRotation)
        .def_property("Scale", &PythonTransformComponent::GetScale, &PythonTransformComponent::SetScale)
        .def_property("LocalPosition", &PythonTransformComponent::GetLocalPosition, &PythonTransformComponent::SetLocalPosition)
        .def_property("LocalRotation", &PythonTransformComponent::GetLocalRotation, &PythonTransformComponent::SetLocalRotation)
        .def_property("LocalScale", &PythonTransformComponent::GetLocalScale, &PythonTransformComponent::SetLocalScale)
        .def_property_readonly("Forward", &PythonTransformComponent::GetForward)
        .def_property_readonly("Right", &PythonTransformComponent::GetRight)
        .def_property_readonly("Up", &PythonTransformComponent::GetUp)
        .def_property("Transform", &PythonTransformComponent::GetTransform, &PythonTransformComponent::SetTransform);

    py::class_<PythonBehaviour, PythonComponent>(m, "Behaviour")
        .def(py::init<>())
        .def_property("ID", &PythonBehaviour::GetID, &PythonBehaviour::SetID)
        .def_property("Enabled", &PythonBehaviour::GetEnabled, &PythonBehaviour::SetEnabled)
        .def_property_readonly("Transform", &PythonBehaviour::GetTransform)
        .def("GetComponent", &PythonBehaviour::GetComponent)
        .def("HasComponent", &PythonBehaviour::HasComponent)
        .def("CreateComponent", &PythonBehaviour::CreateComponent);

    py::class_<PythonAsset>(m, "Asset")
        .def(py::init<uint64_t>())
        .def("__repr__", &PythonAsset::__Repr__);

    py::class_<PythonMesh, PythonAsset>(m, "Mesh")
        .def(py::init<uint64_t>())
        .def(py::init<const char*>())
        .def("__repr__", &PythonMesh::__Repr__)
        .def_property_readonly("_handle", &PythonMesh::GetHandle);
}


// Python Type Registration

namespace Prism
{
    static std::unordered_map<UUID, pybind11::object> s_PythonTypeCache;

    static constexpr uint64_t GenerateTypeID(std::string_view name)
    {
        return Hash::GenerateFNVHash64(name.data());
    }

    void RegisterAllPythonTypes()
    {
        s_PythonTypeCache.clear();
        try
        {
            py::module_ builtins = py::module_::import("builtins");
            py::module_ math = py::module_::import("Prism.Math");
            py::module_ prism = py::module_::import("Prism");
            using namespace Prism::PythonScript;
            s_PythonTypeCache[PYTHON_TYPE_NONE] = py::type::of(py::none());
            s_PythonTypeCache[PYTHON_TYPE_FLOAT] = builtins.attr("float");
            s_PythonTypeCache[PYTHON_TYPE_DOUBLE] = builtins.attr("float");
            s_PythonTypeCache[PYTHON_TYPE_BOOL] = builtins.attr("bool");
            s_PythonTypeCache[PYTHON_TYPE_INT8] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_INT16] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_INT32] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_INT64] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_UINT8] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_UINT16] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_UINT32] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_UINT64] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_VECTOR2] = math.attr("Vector2");
            s_PythonTypeCache[PYTHON_TYPE_VECTOR3] = math.attr("Vector3");
            s_PythonTypeCache[PYTHON_TYPE_VECTOR4] = math.attr("Vector4");
            s_PythonTypeCache[PYTHON_TYPE_OBJECT] = builtins.attr("object");
            s_PythonTypeCache[PYTHON_TYPE_MESHREF] = py::type::of<PythonMesh>();
            //s_PythonTypeCache[PYTHON_TYPE_MATERIALREF] = prism.attr("Material");
            //s_PythonTypeCache[PYTHON_TYPE_TEXTURE2DREF] = prism.attr("Texture2D");
            s_PythonTypeCache[PYTHON_TYPE_ASSET] = py::type::of<PythonAsset>();

            for (const auto& [id, type] : s_PythonTypeCache)
                PR_CORE_INFO("[Python Meta] 注册类型: {} -> {}", id, (std::string)pybind11::str(type));
            
        }
        catch (py::error_already_set& e)
        {
            PR_CORE_WARN("[Python Meta] 注册类型异常: {}", e.what());
            PyErr_Clear();
        }
    }

    void ClearAllPythonTypes()
    {
        s_PythonTypeCache.clear();
    }

    pybind11::object* GetPythonType(const UUID id)
    {
        return s_PythonTypeCache.count(id) ? &s_PythonTypeCache.at(id) : nullptr;
    }

} // namespace Prism::PythonScript
