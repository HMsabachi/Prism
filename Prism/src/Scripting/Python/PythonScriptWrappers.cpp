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

namespace py = pybind11;
using namespace Prism;

namespace Prism
{
    extern std::unordered_map<uint64_t, std::function<void(Entity &)>> s_PythonCreateComponentFuncs;
    extern std::unordered_map<uint64_t, std::function<bool(Entity &)>> s_PythonHasComponentFuncs;
}

#include "PythonScriptTypeCasters.h"

//  Helper

namespace Prism
{
    namespace PythonScript
    {

        static Entity GetEntityFromEntityID(uint64_t entityID)
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            const auto &entityMap = scene->GetEntityMap();
            PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(),
                           "Invalid entity ID or entity doesn't exist in scene!");
            return entityMap.at(entityID);
        }

        //  Region: Log

        void Prism_Log_LogMessage(int32_t level, const char *message)
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

        //  Region: Time

        float Prism_Time_GetDeltaTime() { return Time::GetDeltaTime(); }
        float Prism_Time_GetUnscaledDeltaTime() { return Time::GetUnscaledDeltaTime(); }
        float Prism_Time_GetTime() { return Time::GetTime(); }
        float Prism_Time_GetUnscaledTime() { return Time::GetUnscaledTime(); }
        float Prism_Time_GetFixedDeltaTime() { return Time::GetFixedDeltaTime(); }
        int64_t Prism_Time_GetFrameCount() { return (int64_t)Time::GetFrameCount(); }
        void Prism_Time_SetTimeScale(float scale) { Time::SetTimeScale(scale); }
        float Prism_Time_GetTimeScale() { return Time::GetTimeScale(); }
        void Prism_Time_SetFixedDeltaTime(float fixedDeltaTime) { Time::SetFixedDeltaTime(fixedDeltaTime); }

        //  Region: Math

        float Prism_Noise_PerlinNoise(float x, float y) { return Noise::PerlinNoise(x, y); }

        //  Region: Input

        bool Prism_Input_IsKeyPressed(uint16_t key) { return Input::IsKeyPressed((KeyCode)key); }
        glm::vec2 Prism_Input_GetMousePosition()
        {
            auto [x, y] = Input::GetMousePosition();
            return {x, y};
        }
        void Prism_Input_SetCursorMode(int mode) { Input::SetCursorMode((CursorMode)mode); }
        int Prism_Input_GetCursorMode() { return (int)Input::GetCursorMode(); }
        bool Prism_Input_IsMouseButtonPressed(uint16_t button) { return Input::IsMouseButtonPressed((MouseButton)button); }

        //  Region: Entity

        void Prism_Entity_CreateComponent(uint64_t entityID, py::object cls)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
            s_PythonCreateComponentFuncs.at(typeId)(entity);
        }

        bool Prism_Entity_HasComponent(uint64_t entityID, py::object cls)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
            return s_PythonHasComponentFuncs.at(typeId)(entity);
        }

        uint64_t Prism_Entity_FindEntityByTag(const char *tag)
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            std::string tagStr(tag);
            const auto &entityMap = scene->GetEntityMap();
            for (const auto &[id, entity] : entityMap)
            {
                if (entity.HasComponent<TagComponent>() &&
                    entity.GetComponent<TagComponent>().Tag == tagStr)
                    return id;
            }
            return 0;
        }

        uint64_t Prism_Entity_AddBehaviour(uint64_t entityID, py::object cls)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            std::string className = cls.attr("__module__").cast<std::string>() + "." + cls.attr("__qualname__").cast<std::string>();
            UUID classID = PythonScriptMetaRegistry::GenerateClassID(className);
            auto *ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            UUID behaviourID = ss->AddPythonBehaviour(entity, classID);
            return (uint64_t)behaviourID;
        }

        void Prism_Entity_RemoveBehaviour(uint64_t entityID, uint64_t behaviourID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto *ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            ss->RemovePythonBehaviour(entity, UUID(behaviourID));
        }

        uint64_t Prism_Entity_GetBehaviour(uint64_t entityID, py::object cls)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            std::string className = cls.attr("__module__").cast<std::string>() + "." + cls.attr("__qualname__").cast<std::string>();
            UUID classID = PythonScriptMetaRegistry::GenerateClassID(className);
            auto &comp = entity.GetComponent<PythonScriptComponent>();
            for (auto &[bid, binding] : comp.Behaviours)
            {
                if (binding.ClassID == classID)
                    return (uint64_t)binding.BehaviourID;
            }
            return 0;
        }

        bool Prism_Behaviour_GetEnabled(uint64_t behaviourID)
        {
            auto *ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            return ss->GetEnabled(UUID(behaviourID));
        }

        void Prism_Behaviour_SetEnabled(uint64_t behaviourID, bool enabled)
        {
            auto *ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            ss->SetEnabled(UUID(behaviourID), enabled);
        }

        //  Region: TransformComponent

        static TransformSystem *GetTransformSystem(Entity entity)
        {
            return entity.GetScene()->GetSystem<TransformSystem>();
        }

        glm::vec3 Prism_TransformComponent_GetPosition(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            return GetTransformSystem(entity)->GetWorldPosition(entity);
        }
        glm::vec3 Prism_TransformComponent_GetRotation(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            return GetTransformSystem(entity)->GetWorldRotation(entity);
        }
        glm::vec3 Prism_TransformComponent_GetScale(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            return GetTransformSystem(entity)->GetWorldScale(entity);
        }
        glm::vec3 Prism_TransformComponent_GetUp(uint64_t entityID)
        {
            return GetEntityFromEntityID(entityID).Transformation().Up;
        }
        glm::vec3 Prism_TransformComponent_GetRight(uint64_t entityID)
        {
            return GetEntityFromEntityID(entityID).Transformation().Right;
        }
        glm::vec3 Prism_TransformComponent_GetForward(uint64_t entityID)
        {
            return GetEntityFromEntityID(entityID).Transformation().Forward;
        }

        void Prism_TransformComponent_SetPosition(uint64_t entityID, const glm::vec3 &position)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            GetTransformSystem(entity)->SetWorldPosition(entity, position);
        }
        void Prism_TransformComponent_SetRotation(uint64_t entityID, const glm::vec3 &rotation)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            GetTransformSystem(entity)->SetWorldRotation(entity, rotation);
        }
        void Prism_TransformComponent_SetScale(uint64_t entityID, const glm::vec3 &scale)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            GetTransformSystem(entity)->SetWorldScale(entity, scale);
        }

        ScriptTransform Prism_TransformComponent_GetTransform(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto world = GetTransformSystem(entity)->GetWorldDecomposed(entity);
            auto &tc = entity.Transformation();
            return {world.Position, world.Rotation, world.Scale, tc.Up, tc.Right, tc.Forward};
        }
        void Prism_TransformComponent_SetTransform(uint64_t entityID, const ScriptTransform &transform)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto *ts = GetTransformSystem(entity);
            ts->SetWorldPosition(entity, transform.Position);
            ts->SetWorldRotation(entity, transform.Rotation);
            ts->SetWorldScale(entity, transform.Scale);
        }

        glm::vec3 Prism_TransformComponent_GetLocalPosition(uint64_t entityID)
        {
            return GetEntityFromEntityID(entityID).Transformation().GetPosition();
        }
        void Prism_TransformComponent_SetLocalPosition(uint64_t entityID, const glm::vec3 &position)
        {
            GetEntityFromEntityID(entityID).Transformation().SetPosition(position);
        }
        glm::vec3 Prism_TransformComponent_GetLocalRotation(uint64_t entityID)
        {
            return GetEntityFromEntityID(entityID).Transformation().GetRotation();
        }
        void Prism_TransformComponent_SetLocalRotation(uint64_t entityID, const glm::vec3 &rotation)
        {
            GetEntityFromEntityID(entityID).Transformation().SetRotation(rotation);
        }
        glm::vec3 Prism_TransformComponent_GetLocalScale(uint64_t entityID)
        {
            return GetEntityFromEntityID(entityID).Transformation().GetScale();
        }
        void Prism_TransformComponent_SetLocalScale(uint64_t entityID, const glm::vec3 &scale)
        {
            GetEntityFromEntityID(entityID).Transformation().SetScale(scale);
        }

        //  Region: MeshRendererComponent

        uint64_t Prism_MeshRendererComponent_GetMesh(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto &mc = entity.GetComponent<MeshRendererComponent>();
            return reinterpret_cast<uintptr_t>(new Ref<Mesh>(mc.Mesh));
        }

        void Prism_MeshRendererComponent_SetMesh(uint64_t entityID, uint64_t meshHandle)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto &mc = entity.GetComponent<MeshRendererComponent>();
            mc.Mesh = meshHandle ? *reinterpret_cast<Ref<Mesh> *>(meshHandle) : nullptr;
        }

        uint64_t Prism_MeshRendererComponent_GetMaterial(uint64_t entityID, uint64_t index)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto &mc = entity.GetComponent<MeshRendererComponent>();
            PR_CORE_ASSERT(index < mc.Materials.size(), "Material index out of range");
            return reinterpret_cast<uintptr_t>(new Ref<Material>(mc.Materials[index]));
        }

        void Prism_MeshRendererComponent_SetMaterial(uint64_t entityID, uint64_t materialHandle, uint64_t index)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto &mc = entity.GetComponent<MeshRendererComponent>();
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
                    mc.Materials[i] = *reinterpret_cast<Ref<Material>*>(handles[i]);
                else
                    mc.Materials[i] = nullptr;
            }
        }

        //  Region: Mesh

        uint64_t Prism_Mesh_Constructor(const char *filepath)
        {
            auto result = ModelImporter::Import(filepath);
            return reinterpret_cast<uintptr_t>(new Ref<Mesh>(result.Mesh));
        }

        void Prism_Mesh_Destructor(uint64_t handle)
        {
            delete reinterpret_cast<Ref<Mesh> *>(handle);
        }

        uint64_t Prism_MeshFactory_CreatePlane(float width, float height)
        {
            return reinterpret_cast<uintptr_t>(
                new Ref<Mesh>(ModelImporter::Import("assets/models/Plane1m.obj").Mesh));
        }

        //  Region: Texture2D

        uint64_t Prism_Texture2D_Constructor(uint32_t width, uint32_t height)
        {
            return reinterpret_cast<uintptr_t>(
                new Ref<Texture2D>(Texture2D::Create(TextureFormat::RGBA, width, height)));
        }

        void Prism_Texture2D_Destructor(uint64_t handle)
        {
            delete reinterpret_cast<Ref<Texture2D> *>(handle);
        }

        //  Region: RigidBody2DComponent

        void Prism_RigidBody2DComponent_ApplyLinearImpulse(
            uint64_t entityID, const glm::vec2 &impulse, const glm::vec2 &offset, bool wake)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto &rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body *body = static_cast<b2Body *>(rb2d.RuntimeBody);
            body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y),
                                     b2Vec2(offset.x, offset.y), wake);
        }

        glm::vec2 Prism_RigidBody2DComponent_GetLinearVelocity(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto &rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body *body = static_cast<b2Body *>(rb2d.RuntimeBody);
            const b2Vec2 &v = body->GetLinearVelocity();
            return {v.x, v.y};
        }

        void Prism_RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, const glm::vec2 &velocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto &rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body *body = static_cast<b2Body *>(rb2d.RuntimeBody);
            body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
        }

        //  Region: RigidBodyComponent

        void Prism_RigidBodyComponent_AddForce(uint64_t entityID, const glm::vec3 &force, int32_t forceMode)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
            actor->AddForce(force, (ForceMode)forceMode);
        }
        void Prism_RigidBodyComponent_AddTorque(uint64_t entityID, const glm::vec3 &torque, int32_t forceMode)
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
        void Prism_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, const glm::vec3 &velocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetLinearVelocity(velocity);
        }
        void Prism_RigidBodyComponent_Rotate(uint64_t entityID, const glm::vec3 &rotation)
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
        void Prism_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, const glm::vec3 &velocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetAngularVelocity(velocity);
        }

        //  Region: Physics

        // Shared helper — populates OverlapHitData from PxOverlapHit
        static OverlapHitData FillOverlapHit(physx::PxOverlapHit &pxHit)
        {
            Entity &entity = *(Entity *)pxHit.actor->userData;
            OverlapHitData data{};
            data.EntityID = entity.GetUUID();

            if (entity.HasComponent<BoxColliderComponent>())
            {
                auto &bc = entity.GetComponent<BoxColliderComponent>();
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
                auto &sc = entity.GetComponent<SphereColliderComponent>();
                data.ColliderType = 1;
                data.IsTrigger = sc.IsTrigger;
                data.ShapeData[0] = sc.Radius;
                data.MeshHandle = nullptr;
            }
            else if (entity.HasComponent<CapsuleColliderComponent>())
            {
                auto &cc = entity.GetComponent<CapsuleColliderComponent>();
                data.ColliderType = 2;
                data.IsTrigger = cc.IsTrigger;
                data.ShapeData[0] = cc.Radius;
                data.ShapeData[1] = cc.Height;
                data.MeshHandle = nullptr;
            }
            else if (entity.HasComponent<MeshColliderComponent>())
            {
                auto &mc = entity.GetComponent<MeshColliderComponent>();
                data.ColliderType = 3;
                data.IsTrigger = mc.IsTrigger;
                data.MeshHandle = new Ref<Mesh>(mc.CollisionMesh);
            }
            return data;
        }

        std::optional<RaycastHit> Prism_Physics_Raycast(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance)
        {
            RaycastHit hit;
            if (PXPhysicsWrappers::Raycast(origin, direction, maxDistance, &hit))
                return hit;
            return std::nullopt;
        }

        std::vector<OverlapHitData> Prism_Physics_OverlapBox(const glm::vec3 &origin, const glm::vec3 &halfSize)
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

        std::vector<OverlapHitData> Prism_Physics_OverlapCapsule(const glm::vec3 &origin, float radius, float halfHeight)
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

        std::vector<OverlapHitData> Prism_Physics_OverlapSphere(const glm::vec3 &origin, float radius)
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
        static Ref<Material> &DerefMaterial(uint64_t handle)
        {
            return *reinterpret_cast<Ref<Material> *>(handle);
        }

        uint64_t Prism_Material_Constructor(const char *shaderName)
        {
            const auto &shader = AssetManager::GetShaderLibrary()->Get(shaderName);
            return reinterpret_cast<uintptr_t>(new Ref<Material>(Material::Create(shader->Handle)));
        }

        void Prism_Material_Destructor(uint64_t handle)
        {
            delete reinterpret_cast<Ref<Material> *>(handle);
        }

        void Prism_Material_SetFloat(uint64_t handle, const char *uniform, float value)
        {
            DerefMaterial(handle)->SetFloat(uniform, value);
        }
        void Prism_Material_SetInt(uint64_t handle, const char *uniform, int value)
        {
            DerefMaterial(handle)->SetInt(uniform, value);
        }
        void Prism_Material_SetBool(uint64_t handle, const char *uniform, bool value)
        {
            DerefMaterial(handle)->SetBool(uniform, value);
        }
        void Prism_Material_SetVector2(uint64_t handle, const char *uniform, const glm::vec2 &value)
        {
            DerefMaterial(handle)->SetVec2(uniform, value);
        }
        void Prism_Material_SetVector3(uint64_t handle, const char *uniform, const glm::vec3 &value)
        {
            DerefMaterial(handle)->SetVec3(uniform, value);
        }
        void Prism_Material_SetVector4(uint64_t handle, const char *uniform, const glm::vec4 &value)
        {
            DerefMaterial(handle)->SetVec4(uniform, value);
        }
        void Prism_Material_SetColor3(uint64_t handle, const char *uniform, const glm::vec3 &value)
        {
            DerefMaterial(handle)->SetColor3(uniform, value);
        }
        void Prism_Material_SetColor(uint64_t handle, const char *uniform, const glm::vec4 &value)
        {
            DerefMaterial(handle)->SetColor(uniform, value);
        }
        void Prism_Material_SetMatrix4(uint64_t handle, const char *uniform, const glm::mat4 &value)
        {
            DerefMaterial(handle)->SetMatrix4(uniform, value);
        }
        void Prism_Material_SetTexture(uint64_t handle, const char *uniform, uint64_t textureHandle)
        {
            DerefMaterial(handle)->SetTexture(uniform, *reinterpret_cast<Ref<Texture2D> *>(textureHandle));
        }
        void Prism_Material_SetKeyword(uint64_t handle, const char *name, bool enabled)
        {
            DerefMaterial(handle)->SetKeyword(name, enabled);
        }
        bool Prism_Material_IsKeywordEnabled(uint64_t handle, const char *name)
        {
            return DerefMaterial(handle)->IsKeywordEnabled(name);
        }

    }
} // namespace Prism::PythonScript

