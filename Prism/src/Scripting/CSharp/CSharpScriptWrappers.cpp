#include "prpch.h"
#include "CSharpScriptWrappers.h"
#include "Prism/Core/Math/Noise.h"

#include "Prism/Core/Input.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"

#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Prism/Scene/Systems/ScriptSystem.h"
#include "Scripting/CSharp/CSharpScriptMetaRegistry.h"
#include "Prism/Renderer/Renderer.h"
#include <glm/gtc/type_ptr.hpp>

#include <Rolky/String.hpp>
#include <Rolky/Type.hpp>
#include <Rolky/Array.hpp>

#include <box2d/box2d.h>
#include <PhysX/PxPhysicsAPI.h>

// Node: 在C#端Bool类型为4字节的Rolky::Bool32，而不是C++的bool类型，因此在这里使用Rolky::Bool32来保持一致性。

namespace Prism {
    extern std::unordered_map<Rolky::TypeId, std::function<void(Entity&)>> s_CreateComponentFuncs;
    extern std::unordered_map<Rolky::TypeId, std::function<Rolky::Bool32(Entity&)>> s_HasComponentFuncs;
}

namespace Prism {
    namespace Script
    {

        static Entity GetEntityFromEntityID(uint64_t entityID)
        {
            WeakRef<Scene> scene = CSharpScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            const auto& entityMap = scene->GetEntityMap();
            PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
            return entityMap.at(entityID);
        }


#pragma region Log

        void Prism_Log_LogMessage(LogLevel level, Rolky::String inFormattedMessage)
        {
            std::string message = inFormattedMessage;
            message = "[CSharp]: " + message;
            switch (level)
            {
            case LogLevel::Trace:
                PR_CORE_TRACE(message);
                break;
            case LogLevel::Debug:
                PR_CORE_INFO(message);
                break;
            case LogLevel::Info:
                PR_CORE_INFO(message);
                break;
            case LogLevel::Warn:
                PR_CORE_WARN(message);
                break;
            case LogLevel::Error:
                PR_CORE_ERROR(message);
                break;
            case LogLevel::Critical:
                PR_CORE_FATAL(message);
                break;
            }
            Rolky::String::Free(inFormattedMessage);
        }

#pragma endregion

#pragma region Time
        float Prism_Time_GetDeltaTime(){ return Time::GetDeltaTime(); }
        float Prism_Time_GetUnscaledDeltaTime(){ return Time::GetUnscaledDeltaTime(); }
        float Prism_Time_GetTime(){ return Time::GetTime(); }
        float Prism_Time_GetUnscaledTime(){ return Time::GetUnscaledTime(); }
        float Prism_Time_GetFixedDeltaTime(){ return Time::GetFixedDeltaTime(); }
        int64_t Prism_Time_GetFrameCount(){ return (int64_t)Time::GetFrameCount(); }
        void Prism_Time_SetTimeScale(float scale){ Time::SetTimeScale(scale);}
        float Prism_Time_GetTimeScale(){ return Time::GetTimeScale();}
#pragma endregion

#pragma region Math
        float Prism_Noise_PerlinNoise(float x, float y)
        {
            return Noise::PerlinNoise(x, y);
        }
#pragma endregion

#pragma region Input
        Rolky::Bool32 Prism_Input_IsKeyPressed(KeyCode key)
        {
            return Input::IsKeyPressed(key);
        }
#pragma endregion

#pragma region Entity
        enum class ComponentID
        {
            None = 0,
            Transform = 1,
            Mesh = 2,
            Script = 3,
            SpriteRenderer = 4
        };
        void Prism_Entity_GetTransform(uint64_t entityID, glm::mat4* outTransform)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            memcpy(outTransform, glm::value_ptr(transformComponent.GetTransform()), sizeof(glm::mat4));
        }

        void Prism_Entity_SetTransform(uint64_t entityID, glm::mat4* inTransform)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            transformComponent.SetTransform(*inTransform);
        }

        void Prism_Entity_CreateComponent(uint64_t entityID, Rolky::ReflectionType type)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            Rolky::Type mType = type;
            s_CreateComponentFuncs[mType.GetTypeId()](entity);
        }

        Rolky::Bool32 Prism_Entity_HasComponent(uint64_t entityID, Rolky::ReflectionType type)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            Rolky::Type mType = type;
            Rolky::Bool32 result = s_HasComponentFuncs[mType.GetTypeId()](entity);
            return result;
        }

        uint64_t Prism_Entity_FindEntityByTag(Rolky::String tag)
        {
            WeakRef<Scene> scene = CSharpScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            std::string tagStr = tag;
            Rolky::String::Free(tag);

            const auto& entityMap = scene->GetEntityMap();
            for (const auto& [id, entity] : entityMap)
            {
                if (entity.HasComponent<TagComponent>() && entity.GetComponent<TagComponent>().Tag == tagStr)
                    return id;
            }
            return 0;
        }

        void* Prism_Entity_AddBehaviour(uint64_t entityID, Rolky::String className)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            std::string classNameStr = className;
            Rolky::String::Free(className);

            UUID classID = CSharpScriptMetaRegistry::GenerateClassID(classNameStr);
            auto* ss = CSharpScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            UUID behaviourID = ss->AddCSharpBehaviour(entity, classID);

            UUID sceneID = CSharpScriptEngine::GetCurrentSceneContext()->GetUUID();
            auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, behaviourID);
            PR_CORE_ASSERT(obj && obj->IsValid(), "Failed to get created behaviour instance!");
            return obj->m_Handle;
        }

        void Prism_Entity_RemoveBehaviour(uint64_t entityID, uint64_t behaviourID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            UUID bid(behaviourID);
            auto* ss = CSharpScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            ss->RemoveCSharpBehaviour(entity, bid);
        }

        void* Prism_Entity_GetBehaviour(uint64_t entityID, Rolky::ReflectionType type)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            Rolky::Type mType = type;
            UUID classID = CSharpScriptMetaRegistry::GenerateClassID(mType.GetFullName());

            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            UUID sceneID = CSharpScriptEngine::GetCurrentSceneContext()->GetUUID();
            for (auto& [bid, binding] : comp.Behaviours)
            {
                if (binding.ClassID == classID)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj)
                        return obj->m_Handle;
                }
            }
            return nullptr;
        }

        Rolky::Bool32 Prism_Behaviour_GetEnabled(uint64_t behaviourID)
        {
            auto* ss = CSharpScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            return ss->GetEnabled(UUID(behaviourID));
        }

        void Prism_Behaviour_SetEnabled(uint64_t behaviourID, Rolky::Bool32 enabled)
        {
            auto* ss = CSharpScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            ss->SetEnabled(UUID(behaviourID), enabled);
        }


#pragma endregion

#pragma region TransformComponent
        void Prism_TransformComponent_GetPosition(uint64_t entityID, glm::vec3* outPosition)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            memcpy(outPosition, &transformComponent.Position, sizeof(glm::vec3));
        }
        void Prism_TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            glm::vec3 euler = glm::eulerAngles(transformComponent.Rotation);
            memcpy(outRotation, &euler, sizeof(glm::vec3));
        }

        void Prism_TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            memcpy(outScale, &transformComponent.Scale, sizeof(glm::vec3));
        }
        void Prism_TransformComponent_SetPosition(uint64_t entityID, glm::vec3 inPosition)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            transformComponent.Position = inPosition;
        }

        void Prism_TransformComponent_SetRotation(uint64_t entityID, glm::vec3 inRotation)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            transformComponent.Rotation = glm::quat(glm::radians(inRotation));
        }

        void Prism_TransformComponent_SetScale(uint64_t entityID, glm::vec3 inScale)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            transformComponent.Scale = inScale;
        }

#pragma endregion

#pragma region Mesh
        void* Prism_MeshComponent_GetMesh(uint64_t entityID)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& meshComponent = entity.GetComponent<MeshComponent>();
            return new Ref<Mesh>(meshComponent.Mesh);
        }

        void Prism_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& meshComponent = entity.GetComponent<MeshComponent>();
            meshComponent.Mesh = inMesh ? *inMesh : nullptr;
        }

        Prism::Ref<Prism::Mesh>* Prism_Mesh_Constructor(Rolky::String filepath)
        {
            std::string path = filepath;
            Rolky::String::Free(filepath);
            return new Ref<Mesh>(new Mesh(path));
        }

        void Prism_Mesh_Destructor(Ref<Mesh>* _this)
        {
            Ref<Mesh>* instance = (Ref<Mesh>*)_this;
            delete _this;
        }

        Prism::Ref<Prism::Material>* Prism_Mesh_GetMaterial(Ref<Mesh>* inMesh)
        {
            Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
            return new Ref<Material>(mesh->GetMaterial());
        }

        Prism::Ref<Prism::MaterialInstance>* Prism_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int32_t index)
        {
            Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
            const auto& materials = mesh->GetMaterials();

            PR_CORE_ASSERT(index < materials.size());
            return new Ref<MaterialInstance>(materials[index]);
        }

        int32_t Prism_Mesh_GetMaterialCount(Ref<Mesh>* inMesh)
        {
            Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
            const auto& materials = mesh->GetMaterials();
            return materials.size();
        }

        void Prism_Mesh_SetMaterialByIndex(Ref<Mesh>* inMesh, int32_t index, Ref<MaterialInstance>* material)
        {
            Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
            mesh->SetMaterial(index, material ? *material : nullptr);
        }

        void Prism_Mesh_SetOverrideMaterial(Ref<Mesh>* inMesh, Ref<MaterialInstance>* material)
        {
            Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
            mesh->SetOverrideMaterial(material ? *material : nullptr);
        }

        Prism::Ref<Prism::MaterialInstance>* Prism_Mesh_GetOverrideMaterial(Ref<Mesh>* inMesh)
        {
            Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
            auto overrideMat = mesh->GetOverrideMaterial();
            return overrideMat ? new Ref<MaterialInstance>(overrideMat) : nullptr;
        }

        void* Prism_MeshFactory_CreatePlane(float width, float height)
        {
            return new Ref<Mesh>(new Mesh("assets/models/Plane1m.obj"));
        }

#pragma endregion

#pragma region Texture2D
        void* Prism_Texture2D_Constructor(uint32_t width, uint32_t height)
        {
            auto result = Texture2D::Create(TextureFormat::RGBA, width, height);
            return new Ref<Texture2D>(result);
        }

        void Prism_Texture2D_Destructor(Ref<Texture2D>* _this)
        {
            delete _this;
        }

        void Prism_Texture2D_SetData(Ref<Texture2D>* _this, Rolky::Array<glm::vec4> inData, int32_t count)
        {
            Ref<Texture2D>& instance = *_this;
            uint32_t dataSize = count * sizeof(glm::vec4) / 4;
            instance->Lock();
            Buffer buffer = instance->GetWriteableBuffer();
            PR_CORE_ASSERT(dataSize <= buffer.Size);
            uint8_t* pixels = (uint8_t*)buffer.Data;
            uint32_t index = 0;
            for (int i = 0; i < instance->GetWidth() * instance->GetHeight(); i++)
            {
                glm::vec4& value = inData[i];
                *pixels++ = (uint32_t)(value.x * 255.0f);
                *pixels++ = (uint32_t)(value.y * 255.0f);
                *pixels++ = (uint32_t)(value.z * 255.0f);
                *pixels++ = (uint32_t)(value.w * 255.0f);
            }
            inData.Free(inData);
            instance->Unlock();
        }

#pragma endregion

#pragma region MaterialComponent
    Ref<MaterialInstance>* Prism_MaterialComponent_GetMaterial(uint64_t entityID)
    {
        WeakRef<Scene> scene = CSharpScriptEngine::GetCurrentSceneContext();
        PR_CORE_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& materialComponent = entity.GetComponent<MaterialComponent>();
        return new Ref<MaterialInstance>(materialComponent.Material);
    }

    void Prism_MaterialComponent_SetMaterial(uint64_t entityID, Ref<MaterialInstance>* materialInstance)
    {
        WeakRef<Scene> scene = CSharpScriptEngine::GetCurrentSceneContext();
        PR_CORE_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& materialComponent = entity.GetComponent<MaterialComponent>();
        materialComponent.Material = materialInstance ? *materialInstance : nullptr;
    }

#pragma endregion

#pragma region RigidBody2DComponent
        void Prism_RigidBody2DComponent_ApplyLinearImpulse(uint64_t entityID, glm::vec2* impulse, glm::vec2* offset, Rolky::Bool32 wake)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            b2Vec2 b2Impulse(impulse->x, impulse->y);
            b2Vec2 b2Point(offset->x, offset->y);
            body->ApplyLinearImpulse(b2Impulse, b2Point, wake);
        }

        void Prism_RigidBody2DComponent_GetLinearVelocity(uint64_t entityID, glm::vec2* outVelocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            const b2Vec2& velocity = body->GetLinearVelocity();
            *outVelocity = glm::vec2(velocity.x, velocity.y);
        }

        void Prism_RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, glm::vec2* velocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            body->SetLinearVelocity(b2Vec2(velocity->x, velocity->y));
        }

        #pragma endregion

#pragma region RigidBodyComponent
        void Prism_RigidBodyComponent_AddForce(uint64_t entityID, glm::vec3* force, int32_t forceMode)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb = entity.GetComponent<RigidBodyComponent>();

            if (rb.IsKinematic)
            {
                PR_CORE_WARN("Cannot add a force to a kinematic actor! EntityID({0})", entityID);
                return;
            }

            physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
            physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
            PR_CORE_ASSERT(dynamicActor);
            dynamicActor->addForce(physx::PxVec3(force->x, force->y, force->z), (physx::PxForceMode::Enum)forceMode);
        }

        void Prism_RigidBodyComponent_AddTorque(uint64_t entityID, glm::vec3* torque, int32_t forceMode)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb = entity.GetComponent<RigidBodyComponent>();

            if (rb.IsKinematic)
            {
                PR_CORE_WARN("Cannot add torque to a kinematic actor! EntityID({0})", entityID);
                return;
            }

            physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
            physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
            PR_CORE_ASSERT(dynamicActor);
            dynamicActor->addTorque(physx::PxVec3(torque->x, torque->y, torque->z), (physx::PxForceMode::Enum)forceMode);
        }

        void Prism_RigidBodyComponent_GetLinearVelocity(uint64_t entityID, glm::vec3* outVelocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb = entity.GetComponent<RigidBodyComponent>();
            physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
            physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
            PR_CORE_ASSERT(dynamicActor);
            physx::PxVec3 velocity = dynamicActor->getLinearVelocity();
            *outVelocity = glm::vec3(velocity.x, velocity.y, velocity.z);
        }

        void Prism_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, glm::vec3* velocity)
        {
            Entity entity = GetEntityFromEntityID(entityID);
            auto& rb = entity.GetComponent<RigidBodyComponent>();
            physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
            physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
            PR_CORE_ASSERT(dynamicActor);
            dynamicActor->setLinearVelocity(physx::PxVec3(velocity->x, velocity->y, velocity->z));
        }

#pragma endregion

#pragma region Material
        Ref<Material>* Prism_Material_Constructor(Rolky::String shaderName)
        {
            std::string name = shaderName;
            Rolky::String::Free(shaderName);
            const auto& shader = Renderer::GetShaderLibrary()->Get(name);
            return new Ref<Material>(Material::Create(shader));
        }

        void Prism_Material_Destructor(Ref<Material>* _this)
        {
            delete _this;
        }


        void Prism_Material_SetFloat(Ref<Material>* _this, Rolky::String uniform, float value)
        {
            Ref<Material>& instance = *(Ref<Material>*)_this;
            instance->Set(uniform, value);
            uniform.Free(uniform);
        }

        void Prism_Material_SetTexture(Ref<Material>* _this, Rolky::String uniform, Ref<Texture2D>* texture)
        {
            Ref<Material>& instance = *(Ref<Material>*)_this;
            instance->Set(uniform, *texture);
            uniform.Free(uniform);
        }

        Ref<MaterialInstance>* Prism_MaterialInstance_Constructor(Ref<Material>* parent)
        {
            Ref<Material>& material = *(Ref<Material>*)parent;
            return new Ref<MaterialInstance>(MaterialInstance::Create(material));
        }

        void Prism_MaterialInstance_Destructor(Ref<MaterialInstance>* _this)
        {
            delete _this;
        }

        void Prism_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, Rolky::String uniform, float value)
        {
            Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
            instance->Set(uniform, value);
            uniform.Free(uniform);
        }

        void Prism_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, Rolky::String uniform, glm::vec3* value)
        {
            Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
            instance->Set(uniform, *value);
            uniform.Free(uniform);
        }

        void Prism_MaterialInstance_SetVector4(Ref<MaterialInstance>* _this, Rolky::String uniform, glm::vec4* value)
        {
            Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
            instance->Set(uniform, *value);
            uniform.Free(uniform);
        }

        void Prism_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, Rolky::String uniform, Ref<Texture2D>* texture)
        {
            Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
            instance->Set(uniform, *texture);
            uniform.Free(uniform);
        }

        void Prism_Material_SetKeyword(Ref<Material>* _this, Rolky::String name, Rolky::Bool32 enabled)
        {
            Ref<Material>& instance = *(Ref<Material>*)_this;
            std::string kwName = name;
            name.Free(name);
            instance->SetKeyword(kwName, enabled);
        }

        Rolky::Bool32 Prism_Material_IsKeywordEnabled(Ref<Material>* _this, Rolky::String name)
        {
            Ref<Material>& instance = *(Ref<Material>*)_this;
            std::string kwName = name;
            name.Free(name);
            return instance->IsKeywordEnabled(kwName);
        }

        void Prism_MaterialInstance_SetKeyword(Ref<MaterialInstance>* _this, Rolky::String name, Rolky::Bool32 enabled)
        {
            Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
            std::string kwName = name;
            name.Free(name);
            instance->SetKeyword(kwName, enabled);
        }

        Rolky::Bool32 Prism_MaterialInstance_IsKeywordEnabled(Ref<MaterialInstance>* _this, Rolky::String name)
        {
            Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
            std::string kwName = name;
            name.Free(name);
            return instance->IsKeywordEnabled(kwName);
        }

#pragma endregion


    }
}
