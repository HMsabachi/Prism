#include "prpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"

#include "yaml-cpp/yaml.h"

#include "Prism/Renderer/MeshFactory.h"
#include "Prism/Physics/PhysicsLayer.h"
#include "Prism/Physics/PXPhysicsWrappers.h"
#include "Prism/Utilities/FileSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Prism/Core/Hash.h>
#include "Scripting/CSharp/CSharpScriptMetaRegistry.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"
#include "Prism/Scene/Systems/ScriptSystem.h"

#include <iostream>
#include <fstream>

namespace YAML {

    template<>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::quat>
    {
        static Node encode(const glm::quat& rhs)
        {
            Node node;
            node.push_back(rhs.w);
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::quat& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.w = node[0].as<float>();
            rhs.x = node[1].as<float>();
            rhs.y = node[2].as<float>();
            rhs.z = node[3].as<float>();
            return true;
        }
    };
}

namespace Prism {

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }


    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        : m_Scene(scene)
    {
    }

        // ── FNV-1a hash for field name lookup ──
    static uint32_t HashFieldName(const std::string& name)
    {
        return (uint32_t)Hash::GenerateFNVHash64(name);
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity, Scene* scene)
    {
        UUID uuid = entity.GetComponent<IDComponent>().ID;
        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity";
        out << YAML::Value << uuid;

        if (entity.HasComponent<TagComponent>())
        {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent

            auto& tag = entity.GetComponent<TagComponent>().Tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap; // TagComponent
        }

        if (entity.HasComponent<TransformComponent>())
        {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TransformComponent

            auto& tc = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Position" << YAML::Value << tc.Position;
            out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

            out << YAML::EndMap; // TransformComponent
        }

        if (entity.HasComponent<MeshComponent>())
        {
            out << YAML::Key << "MeshComponent";
            out << YAML::BeginMap; // MeshComponent

            auto mesh = entity.GetComponent<MeshComponent>().Mesh;
            out << YAML::Key << "AssetPath" << YAML::Value << FileSystem::GetRelativePath(mesh->GetFilePath());

            out << YAML::EndMap; // MeshComponent
        }
        if (entity.HasComponent<MaterialComponent>())
        {
            out << YAML::Key << "MaterialComponent";
            out << YAML::BeginMap; // MaterialComponent
            out << YAML::EndMap; // MaterialComponent
        }

        if (entity.HasComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap; // CameraComponent

            auto& cameraComponent = entity.GetComponent<CameraComponent>();
            out << YAML::Key << "Camera" << YAML::Value << "some camera data...";
            out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;

            out << YAML::EndMap; // CameraComponent
        }

        if (entity.HasComponent<SpriteRendererComponent>())
        {
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap; // SpriteRendererComponent

            auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
            if (spriteRendererComponent.Texture)
                out << YAML::Key << "TextureAssetPath" << YAML::Value << "path/to/asset";
            out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;

            out << YAML::EndMap; // SpriteRendererComponent
        }

        if (entity.HasComponent<RigidBody2DComponent>())
        {
            out << YAML::Key << "RigidBody2DComponent";
            out << YAML::BeginMap; // RigidBody2DComponent

            auto& rb2dComponent = entity.GetComponent<RigidBody2DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << (int)rb2dComponent.BodyType;
            out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

            out << YAML::EndMap; // RigidBody2DComponent
        }

        if (entity.HasComponent<BoxCollider2DComponent>())
        {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap; // BoxCollider2DComponent

            auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
            out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
            out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
            out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;

            out << YAML::EndMap; // BoxCollider2DComponent
        }

        if (entity.HasComponent<CircleCollider2DComponent>())
        {
            out << YAML::Key << "CircleCollider2DComponent";
            out << YAML::BeginMap; // CircleCollider2DComponent

            auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
            out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
            out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
            out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;

            out << YAML::EndMap; // CircleCollider2DComponent
        }

        if (entity.HasComponent<RigidBodyComponent>())
        {
            out << YAML::Key << "RigidBodyComponent";
            out << YAML::BeginMap; // RigidBodyComponent

            auto& rbComponent = entity.GetComponent<RigidBodyComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << (int)rbComponent.BodyType;
            out << YAML::Key << "Mass" << YAML::Value << rbComponent.Mass;
            out << YAML::Key << "IsKinematic" << YAML::Value << rbComponent.IsKinematic;
            out << YAML::Key << "Layer" << YAML::Value << rbComponent.Layer;
            out << YAML::Key << "Constraints";
            out << YAML::BeginMap; // Constraints
            out << YAML::Key << "LockPositionX" << YAML::Value << rbComponent.LockPositionX;
            out << YAML::Key << "LockPositionY" << YAML::Value << rbComponent.LockPositionY;
            out << YAML::Key << "LockPositionZ" << YAML::Value << rbComponent.LockPositionZ;
            out << YAML::Key << "LockRotationX" << YAML::Value << rbComponent.LockRotationX;
            out << YAML::Key << "LockRotationY" << YAML::Value << rbComponent.LockRotationY;
            out << YAML::Key << "LockRotationZ" << YAML::Value << rbComponent.LockRotationZ;
            out << YAML::EndMap; // Constraints

            out << YAML::EndMap; // RigidBodyComponent
        }

        if (entity.HasComponent<PhysicsMaterialComponent>())
        {
            out << YAML::Key << "PhysicsMaterialComponent";
            out << YAML::BeginMap; // PhysicsMaterialComponent

            auto& pmComponent = entity.GetComponent<PhysicsMaterialComponent>();
            out << YAML::Key << "StaticFriction" << YAML::Value << pmComponent.StaticFriction;
            out << YAML::Key << "DynamicFriction" << YAML::Value << pmComponent.DynamicFriction;
            out << YAML::Key << "Bounciness" << YAML::Value << pmComponent.Bounciness;

            out << YAML::EndMap; // PhysicsMaterialComponent
        }

        if (entity.HasComponent<BoxColliderComponent>())
        {
            out << YAML::Key << "BoxColliderComponent";
            out << YAML::BeginMap; // BoxColliderComponent

            auto& bcComponent = entity.GetComponent<BoxColliderComponent>();
            out << YAML::Key << "Size" << YAML::Value << bcComponent.Size;
            out << YAML::Key << "Offset" << YAML::Value << bcComponent.Offset;
            out << YAML::Key << "IsTrigger" << YAML::Value << bcComponent.IsTrigger;

            out << YAML::EndMap; // BoxColliderComponent
        }

        if (entity.HasComponent<SphereColliderComponent>())
        {
            out << YAML::Key << "SphereColliderComponent";
            out << YAML::BeginMap; // SphereColliderComponent

            auto& scComponent = entity.GetComponent<SphereColliderComponent>();
            out << YAML::Key << "Radius" << YAML::Value << scComponent.Radius;
            out << YAML::Key << "IsTrigger" << YAML::Value << scComponent.IsTrigger;

            out << YAML::EndMap; // SphereColliderComponent
        }

        if (entity.HasComponent<CapsuleColliderComponent>())
        {
            out << YAML::Key << "CapsuleColliderComponent";
            out << YAML::BeginMap; // CapsuleColliderComponent

            auto& ccComponent = entity.GetComponent<CapsuleColliderComponent>();
            out << YAML::Key << "Radius" << YAML::Value << ccComponent.Radius;
            out << YAML::Key << "Height" << YAML::Value << ccComponent.Height;
            out << YAML::Key << "IsTrigger" << YAML::Value << ccComponent.IsTrigger;

            out << YAML::EndMap; // CapsuleColliderComponent
        }

        if (entity.HasComponent<MeshColliderComponent>())
        {
            out << YAML::Key << "MeshColliderComponent";
            out << YAML::BeginMap; // MeshColliderComponent

            auto& mcComponent = entity.GetComponent<MeshColliderComponent>();
            if (mcComponent.CollisionMesh)
                out << YAML::Key << "AssetPath" << YAML::Value << FileSystem::GetRelativePath(mcComponent.CollisionMesh->GetFilePath());
            out << YAML::Key << "IsTrigger" << YAML::Value << mcComponent.IsTrigger;

            out << YAML::EndMap; // MeshColliderComponent
        }

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            out << YAML::Key << "CSharpScriptComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "Behaviours";
            out << YAML::BeginSeq;
            for (auto& [bid, binding] : comp.Behaviours)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "ClassID" << YAML::Value << (uint64_t)binding.ClassID;
                out << YAML::Key << "Enabled" << YAML::Value << binding.Enabled;
                if (!binding.Fields.empty())
                {
                    out << YAML::Key << "Fields";
                    out << YAML::BeginSeq;
                    for (auto& [fieldID, field] : binding.Fields)
                    {
                        out << YAML::BeginMap;
                        out << YAML::Key << "ID" << YAML::Value << fieldID;
                        out << YAML::Key << "Name" << YAML::Value << field.GetName();
                        out << YAML::Key << "Type" << YAML::Value << (uint16_t)field.GetType();
                        out << YAML::Key << "Value" << YAML::Value;
                        switch (field.GetType())
                        {
                        case ScriptFieldType::Float:
                            out << field.GetValue<float>();
                            break;
                        case ScriptFieldType::Double:
                            out << field.GetValue<double>();
                            break;
                        case ScriptFieldType::Bool:
                            out << field.GetValue<bool>();
                            break;
                        case ScriptFieldType::Int8:
                            out << field.GetValue<int8_t>();
                            break;
                        case ScriptFieldType::Int16:
                            out << field.GetValue<int16_t>();
                            break;
                        case ScriptFieldType::Int32:
                            out << field.GetValue<int32_t>();
                            break;
                        case ScriptFieldType::Int64:
                            out << field.GetValue<int64_t>();
                            break;
                        case ScriptFieldType::UInt8:
                            out << field.GetValue<uint8_t>();
                            break;
                        case ScriptFieldType::UInt16:
                            out << field.GetValue<uint16_t>();
                            break;
                        case ScriptFieldType::UInt32:
                            out << field.GetValue<uint32_t>();
                            break;
                        case ScriptFieldType::UInt64:
                            out << field.GetValue<uint64_t>();
                            break;
                        case ScriptFieldType::Vector2:
                            out << field.GetValue<glm::vec2>();
                            break;
                        case ScriptFieldType::Vector3:
                            out << field.GetValue<glm::vec3>();
                            break;
                        case ScriptFieldType::Vector4:
                            out << field.GetValue<glm::vec4>();
                            break;
                        case ScriptFieldType::Object:
                            // TODO: Serialize object references (e.g. assets, entities)
                            break;
                        }
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }

        if (entity.HasComponent<PythonScriptComponent>())
        {
            auto& comp = entity.GetComponent<PythonScriptComponent>();
            out << YAML::Key << "PythonScriptComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "Behaviours";
            out << YAML::BeginSeq;
            for (auto& [bid, binding] : comp.Behaviours)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "ClassID" << YAML::Value << (uint64_t)binding.ClassID;
                out << YAML::Key << "Enabled" << YAML::Value << binding.Enabled;
                if (!binding.Fields.empty())
                {
                    out << YAML::Key << "Fields";
                    out << YAML::BeginSeq;
                    for (auto& [fieldID, field] : binding.Fields)
                    {
                        out << YAML::BeginMap;
                        out << YAML::Key << "ID" << YAML::Value << fieldID;
                        out << YAML::Key << "Name" << YAML::Value << field.GetName();
                        out << YAML::Key << "Type" << YAML::Value << (uint16_t)field.GetType();
                        out << YAML::Key << "Value" << YAML::Value;
                        switch (field.GetType())
                        {
                        case ScriptFieldType::Float:
                            out << field.GetValue<float>();
                            break;
                        case ScriptFieldType::Double:
                            out << field.GetValue<double>();
                            break;
                        case ScriptFieldType::Bool:
                            out << field.GetValue<bool>();
                            break;
                        case ScriptFieldType::Int8:
                            out << field.GetValue<int8_t>();
                            break;
                        case ScriptFieldType::Int16:
                            out << field.GetValue<int16_t>();
                            break;
                        case ScriptFieldType::Int32:
                            out << field.GetValue<int32_t>();
                            break;
                        case ScriptFieldType::Int64:
                            out << field.GetValue<int64_t>();
                            break;
                        case ScriptFieldType::UInt8:
                            out << field.GetValue<uint8_t>();
                            break;
                        case ScriptFieldType::UInt16:
                            out << field.GetValue<uint16_t>();
                            break;
                        case ScriptFieldType::UInt32:
                            out << field.GetValue<uint32_t>();
                            break;
                        case ScriptFieldType::UInt64:
                            out << field.GetValue<uint64_t>();
                            break;
                        case ScriptFieldType::Vector2:
                            out << field.GetValue<glm::vec2>();
                            break;
                        case ScriptFieldType::Vector3:
                            out << field.GetValue<glm::vec3>();
                            break;
                        case ScriptFieldType::Vector4:
                            out << field.GetValue<glm::vec4>();
                            break;
                        case ScriptFieldType::Object:
                            // TODO: Serialize object references (e.g. assets, entities)
                            break;
                        }
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }

        out << YAML::EndMap; // Entity
    }

    static void SerializeEnvironment(YAML::Emitter& out, const Ref<Scene>& scene)
    {
        out << YAML::Key << "Environment";
        out << YAML::Value;
        out << YAML::BeginMap; // Environment
        out << YAML::Key << "AssetPath" << YAML::Value << FileSystem::GetRelativePath(scene->GetEnvironment().FilePath);
        const auto& light = scene->GetLight();
        out << YAML::Key << "Light" << YAML::Value;
        out << YAML::BeginMap; // Light
        out << YAML::Key << "Direction" << YAML::Value << light.Direction;
        out << YAML::Key << "Radiance" << YAML::Value << light.Radiance;
        out << YAML::Key << "Multiplier" << YAML::Value << light.Multiplier;
        out << YAML::EndMap; // Light
        out << YAML::Key << "Shadow" << YAML::Value;
        out << YAML::BeginMap; // Shadow
        out << YAML::Key << "Enabled" << YAML::Value << scene->IsShadowEnabled();
        out << YAML::Key << "Bias" << YAML::Value << scene->GetShadowBias();
        out << YAML::Key << "NormalBias" << YAML::Value << scene->GetShadowNormalBias();
        out << YAML::Key << "CascadeCount" << YAML::Value << scene->GetCascadeCount();
        out << YAML::EndMap; // Shadow
        out << YAML::EndMap; // Environment
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene";
        out << YAML::Value << "Scene Name";
        SerializeEnvironment(out, m_Scene);

        out << YAML::Key << "PhysicsLayers";
        out << YAML::Value << YAML::BeginSeq;
        for (uint32_t i = 0; i < PhysicsLayerManager::GetLayerCount(); i++)
        {
            const PhysicsLayer& layer = PhysicsLayerManager::GetLayer(i);

            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << layer.Name;

            out << YAML::Key << "CollidesWith" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& collisionLayer : PhysicsLayerManager::GetLayerCollisions(layer.LayerID))
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Name" << YAML::Value << collisionLayer.Name;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::Key << "Entities";
        out << YAML::Value << YAML::BeginSeq;
        for (auto entityID : m_Scene->m_Registry.view<entt::entity>())
        {
            Entity entity = { entityID, m_Scene.Raw() };
            if (!entity || !entity.HasComponent<IDComponent>())
                continue;
            SerializeEntity(out, entity, m_Scene.Raw());
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SceneSerializer::SerializeRuntime(const std::string& filepath)
    {
        // Not implemented
        PR_CORE_ASSERT(false);
    }

    bool SceneSerializer::Deserialize(const std::string& filepath)
    {
        std::ifstream stream(filepath);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Scene"])
            return false;

        std::string sceneName = data["Scene"].as<std::string>();
        PR_CORE_INFO("Deserializing scene '{0}'", sceneName);

        auto environment = data["Environment"];
        if (environment)
        {
            std::string envPath = environment["AssetPath"].as<std::string>();
            m_Scene->SetEnvironment(Environment::Load(envPath));

            auto lightNode = environment["Light"];
            if (lightNode)
            {
                auto& light = m_Scene->GetLight();
                light.Direction = lightNode["Direction"].as<glm::vec3>();
                light.Radiance = lightNode["Radiance"].as<glm::vec3>();
                light.Multiplier = lightNode["Multiplier"].as<float>();
            }
            auto shadowNode = environment["Shadow"];
            if (shadowNode)
            {
                m_Scene->SetShadowEnabled(shadowNode["Enabled"].as<bool>());
                m_Scene->SetShadowBias(shadowNode["Bias"].as<float>());
                m_Scene->SetShadowNormalBias(shadowNode["NormalBias"].as<float>());
                m_Scene->SetCascadeCount(shadowNode["CascadeCount"].as<uint32_t>());
            }
        }

        auto physicsLayers = data["PhysicsLayers"];
        if (physicsLayers)
        {
            PhysicsLayerManager::ClearLayers();

            for (auto layer : physicsLayers)
            {
                PhysicsLayerManager::AddLayer(layer["Name"].as<std::string>(), false);
            }

            for (auto layer : physicsLayers)
            {
                const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(layer["Name"].as<std::string>());

                auto collidesWith = layer["CollidesWith"];
                if (collidesWith)
                {
                    for (auto collisionLayer : collidesWith)
                    {
                        const auto& otherLayer = PhysicsLayerManager::GetLayer(collisionLayer["Name"].as<std::string>());
                        PhysicsLayerManager::SetLayerCollision(layerInfo.LayerID, otherLayer.LayerID, true);
                    }
                }
            }
        }

        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>();

                std::string name;
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                PR_CORE_INFO("Deserialized entity with ID = {0}, name = {1}", uuid, name);

                Entity deserializedEntity = m_Scene->CreateEntityWithID(uuid, name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent)
                {
                    // Entities always have transforms
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    glm::vec3 translation = transformComponent["Position"].as<glm::vec3>();
                    glm::quat rotation = transformComponent["Rotation"].as<glm::quat>();
                    glm::vec3 scale = transformComponent["Scale"].as<glm::vec3>();

                    tc.Position = translation;
                    tc.Rotation = rotation;
                    tc.Scale = scale;

                    PR_CORE_INFO("  Entity Transform:");
                    PR_CORE_INFO("    Translation: {0}, {1}, {2}", translation.x, translation.y, translation.z);
                    PR_CORE_INFO("    Rotation: {0}, {1}, {2}, {3}", rotation.w, rotation.x, rotation.y, rotation.z);
                    PR_CORE_INFO("    Scale: {0}, {1}, {2}", scale.x, scale.y, scale.z);
                }

                // Script serialization is not yet implemented for the new
                // CSharpScriptComponent / PythonScriptComponent system.
                // TODO: Serialize Behaviour list and field values via engine API.
                // Backward compat: read old ScriptsComponent / ScriptComponent format
                // to avoid data loss, but data is not processed (FieldType and
                // ScriptStorage have been removed).
                {
                    auto scriptsNode = entity["ScriptsComponent"];
                    if (scriptsNode)
                    {
                        for (auto scriptNode : scriptsNode)
                        {
                            std::string moduleName = scriptNode["ModuleName"].as<std::string>();
                            PR_CORE_INFO("  Script (legacy): Module={0}", moduleName);
                        }
                    }
                    else
                    {
                        auto scriptComponent = entity["ScriptComponent"];
                        if (scriptComponent)
                        {
                            std::string moduleName = scriptComponent["ModuleName"].as<std::string>();
                            PR_CORE_INFO("  Script Module (legacy): {0}", moduleName);
                        }
                    }
                }

                auto meshComponent = entity["MeshComponent"];
                if (meshComponent)
                {
                    std::string meshPath = meshComponent["AssetPath"].as<std::string>();
                    // TEMP (because script creates mesh component...)
                    if (!deserializedEntity.HasComponent<MeshComponent>())
                        deserializedEntity.AddComponent<MeshComponent>(Ref<Mesh>::Create(meshPath));

                    PR_CORE_INFO("  Mesh Asset Path: {0}", meshPath);
                }

                auto materialComponent = entity["MaterialComponent"];
                if (materialComponent)
                {
                    deserializedEntity.AddComponent<MaterialComponent>();
                    PR_CORE_INFO("  MaterialComponent present.");
                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent)
                {
                    auto& component = deserializedEntity.AddComponent<CameraComponent>();
                    component.Camera = SceneCamera();
                    component.Primary = cameraComponent["Primary"].as<bool>();

                    PR_CORE_INFO("  Primary Camera: {0}", component.Primary);
                }

                auto spriteRendererComponent = entity["SpriteRendererComponent"];
                if (spriteRendererComponent)
                {
                    auto& component = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    component.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                    component.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();

                    PR_CORE_INFO("  SpriteRendererComponent present.");
                }

                auto rigidBody2DComponent = entity["RigidBody2DComponent"];
                if (rigidBody2DComponent)
                {
                    auto& component = deserializedEntity.AddComponent<RigidBody2DComponent>();
                    component.BodyType = (RigidBody2DComponent::Type)rigidBody2DComponent["BodyType"].as<int>();
                    component.FixedRotation = rigidBody2DComponent["FixedRotation"] ? rigidBody2DComponent["FixedRotation"].as<bool>() : false;

                    PR_CORE_INFO("  RigidBody2DComponent: Type={0}, FixedRotation={1}", (int)component.BodyType, component.FixedRotation);
                }

                auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
                if (boxCollider2DComponent)
                {
                    auto& component = deserializedEntity.AddComponent<BoxCollider2DComponent>();
                    component.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
                    component.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
                    component.Density = boxCollider2DComponent["Density"] ? boxCollider2DComponent["Density"].as<float>() : 1.0f;
                    component.Friction = boxCollider2DComponent["Friction"] ? boxCollider2DComponent["Friction"].as<float>() : 1.0f;

                    PR_CORE_INFO("  BoxCollider2DComponent: Offset={0},{1}, Size={2},{3}, Density={4}, Friction={5}", component.Offset.x, component.Offset.y, component.Size.x, component.Size.y, component.Density, component.Friction);
                }

                auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
                if (circleCollider2DComponent)
                {
                    auto& component = deserializedEntity.AddComponent<CircleCollider2DComponent>();
                    component.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
                    component.Radius = circleCollider2DComponent["Radius"].as<float>();
                    component.Density = circleCollider2DComponent["Density"] ? circleCollider2DComponent["Density"].as<float>() : 1.0f;
                    component.Friction = circleCollider2DComponent["Friction"] ? circleCollider2DComponent["Friction"].as<float>() : 1.0f;

                    PR_CORE_INFO("  CircleCollider2DComponent: Offset={0},{1}, Radius={2}, Density={3}, Friction={4}", component.Offset.x, component.Offset.y, component.Radius, component.Density, component.Friction);
                }
                auto rigidBodyComponent = entity["RigidBodyComponent"];
                if (rigidBodyComponent)
                {
                    auto& component = deserializedEntity.AddComponent<RigidBodyComponent>();
                    component.BodyType = (RigidBodyComponent::Type)rigidBodyComponent["BodyType"].as<int>();
                    component.Mass = rigidBodyComponent["Mass"] ? rigidBodyComponent["Mass"].as<float>() : 1.0f;
                    component.IsKinematic = rigidBodyComponent["IsKinematic"] ? rigidBodyComponent["IsKinematic"].as<bool>() : false;
                    component.Layer = rigidBodyComponent["Layer"] ? rigidBodyComponent["Layer"].as<uint32_t>() : 0;
                    component.LockPositionX = rigidBodyComponent["Constraints"]["LockPositionX"].as<bool>();
                    component.LockPositionY = rigidBodyComponent["Constraints"]["LockPositionY"].as<bool>();
                    component.LockPositionZ = rigidBodyComponent["Constraints"]["LockPositionZ"].as<bool>();
                    component.LockRotationX = rigidBodyComponent["Constraints"]["LockRotationX"].as<bool>();
                    component.LockRotationY = rigidBodyComponent["Constraints"]["LockRotationY"].as<bool>();
                    component.LockRotationZ = rigidBodyComponent["Constraints"]["LockRotationZ"].as<bool>();

                    PR_CORE_INFO("  RigidBodyComponent: Type={0}, Mass={1}", (int)component.BodyType, component.Mass);
                }

                auto physicsMaterialComponent = entity["PhysicsMaterialComponent"];
                if (physicsMaterialComponent)
                {
                    auto& component = deserializedEntity.AddComponent<PhysicsMaterialComponent>();
                    component.StaticFriction = physicsMaterialComponent["StaticFriction"].as<float>();
                    component.DynamicFriction = physicsMaterialComponent["DynamicFriction"].as<float>();
                    component.Bounciness = physicsMaterialComponent["Bounciness"].as<float>();

                    PR_CORE_INFO("  PhysicsMaterialComponent: StaticFriction={0}, DynamicFriction={1}, Bounciness={2}", component.StaticFriction, component.DynamicFriction, component.Bounciness);
                }

                auto boxColliderComponent = entity["BoxColliderComponent"];
                if (boxColliderComponent)
                {
                    auto& component = deserializedEntity.AddComponent<BoxColliderComponent>();
                    component.Size = boxColliderComponent["Size"].as<glm::vec3>();
                    component.Offset = boxColliderComponent["Offset"].as<glm::vec3>();
                    component.IsTrigger = boxColliderComponent["IsTrigger"] ? boxColliderComponent["IsTrigger"].as<bool>() : false;
                    component.DebugMesh = MeshFactory::CreateBox(component.Size);

                    PR_CORE_INFO("  BoxColliderComponent: Size={0},{1},{2}, Offset={3},{4},{5}", component.Size.x, component.Size.y, component.Size.z, component.Offset.x, component.Offset.y, component.Offset.z);
                }

                auto sphereColliderComponent = entity["SphereColliderComponent"];
                if (sphereColliderComponent)
                {
                    auto& component = deserializedEntity.AddComponent<SphereColliderComponent>();
                    component.Radius = sphereColliderComponent["Radius"].as<float>();
                    component.IsTrigger = sphereColliderComponent["IsTrigger"] ? sphereColliderComponent["IsTrigger"].as<bool>() : false;
                    component.DebugMesh = MeshFactory::CreateSphere(component.Radius);

                    PR_CORE_INFO("  SphereColliderComponent: Radius={0}", component.Radius);
                }

                auto capsuleColliderComponent = entity["CapsuleColliderComponent"];
                if (capsuleColliderComponent)
                {
                    auto& component = deserializedEntity.AddComponent<CapsuleColliderComponent>();
                    component.Radius = capsuleColliderComponent["Radius"].as<float>();
                    component.Height = capsuleColliderComponent["Height"].as<float>();
                    component.IsTrigger = capsuleColliderComponent["IsTrigger"] ? capsuleColliderComponent["IsTrigger"].as<bool>() : false;
                    component.DebugMesh = MeshFactory::CreateCapsule(component.Radius, component.Height);

                    PR_CORE_INFO("  CapsuleColliderComponent: Radius={0}, Height={1}", component.Radius, component.Height);
                }

                auto meshColliderComponent = entity["MeshColliderComponent"];
                if (meshColliderComponent)
                {
                    std::string meshPath = meshColliderComponent["AssetPath"].as<std::string>();
                    auto& component = deserializedEntity.AddComponent<MeshColliderComponent>(Ref<Mesh>::Create(meshPath));
                    component.IsTrigger = meshColliderComponent["IsTrigger"] ? meshColliderComponent["IsTrigger"].as<bool>() : false;
                    PXPhysicsWrappers::CreateConvexMesh(component);

                    PR_CORE_INFO("  MeshColliderComponent: AssetPath={0}", FileSystem::GetRelativePath(meshPath));
                }

                // CSharpScriptComponent — deserialize Behaviours
                auto csharpScriptComponent = entity["CSharpScriptComponent"];
                if (csharpScriptComponent)
                {
                    auto& comp = deserializedEntity.GetComponent<CSharpScriptComponent>();
                    auto behavioursNode = csharpScriptComponent["Behaviours"];
                    if (behavioursNode)
                    {
                        for (auto bindingNode : behavioursNode)
                        {
                            UUID classID;
                            if (bindingNode["ClassID"])
                                classID = (UUID)bindingNode["ClassID"].as<uint64_t>();
                            else if (bindingNode["ClassName"])
                            {
                                std::string className = bindingNode["ClassName"].as<std::string>();
                                classID = CSharpScriptMetaRegistry::GenerateClassID(className);
                            }

                            auto* ss = m_Scene->GetSystem<ScriptSystem>();
                            auto binding = ss->CreateCSharpBinding(classID);
                            if (bindingNode["Enabled"])
                                binding.Enabled = bindingNode["Enabled"].as<bool>();

                            auto fieldsNode = bindingNode["Fields"];
                            if (fieldsNode)
                            {
                                for (auto fieldNode : fieldsNode)
                                {
                                    std::string fieldName = fieldNode["Name"].as<std::string>();
                                    ScriptFieldType fieldType = (ScriptFieldType)fieldNode["Type"].as<uint16_t>();
                                    uint32_t fieldHash = HashFieldName(fieldName);

                                    auto it = binding.Fields.find(fieldHash);
                                    if (it != binding.Fields.end() && it->second.GetType() == fieldType && fieldNode["Value"])
                                    {
                                        switch (fieldType)
                                        {
                                        case ScriptFieldType::Float:
                                            it->second.SetValue(fieldNode["Value"].as<float>());
                                            break;
                                        case ScriptFieldType::Double:
                                            it->second.SetValue(fieldNode["Value"].as<double>());
                                            break;
                                        case ScriptFieldType::Bool:
                                            it->second.SetValue(fieldNode["Value"].as<bool>());
                                            break;
                                        case ScriptFieldType::Int8:
                                            it->second.SetValue(fieldNode["Value"].as<int8_t>());
                                            break;
                                        case ScriptFieldType::Int16:
                                            it->second.SetValue(fieldNode["Value"].as<int16_t>());
                                            break;
                                        case ScriptFieldType::Int32:
                                            it->second.SetValue(fieldNode["Value"].as<int32_t>());
                                            break;
                                        case ScriptFieldType::Int64:
                                            it->second.SetValue(fieldNode["Value"].as<int64_t>());
                                            break;
                                        case ScriptFieldType::UInt8:
                                            it->second.SetValue(fieldNode["Value"].as<uint8_t>());
                                            break;
                                        case ScriptFieldType::UInt16:
                                            it->second.SetValue(fieldNode["Value"].as<uint16_t>());
                                            break;
                                        case ScriptFieldType::UInt32:
                                            it->second.SetValue(fieldNode["Value"].as<uint32_t>());
                                            break;
                                        case ScriptFieldType::UInt64:
                                            it->second.SetValue(fieldNode["Value"].as<uint64_t>());
                                            break;
                                        case ScriptFieldType::Vector2:
                                            it->second.SetValue(fieldNode["Value"].as<glm::vec2>());
                                            break;
                                        case ScriptFieldType::Vector3:
                                            it->second.SetValue(fieldNode["Value"].as<glm::vec3>());
                                            break;
                                        case ScriptFieldType::Vector4:
                                            it->second.SetValue(fieldNode["Value"].as<glm::vec4>());
                                            break;
                                        case ScriptFieldType::Object:
                                            break;
                                        }
                                    }
                                }
                            }

                            ss->RegisterCSharpBinding(deserializedEntity, std::move(binding));
                        }
                    }

                    PR_CORE_INFO("  CSharpScriptComponent: {0} Behaviours loaded", comp.Behaviours.size());
                }

                // PythonScriptComponent — deserialize Behaviours
                auto pythonScriptComponent = entity["PythonScriptComponent"];
                if (pythonScriptComponent)
                {
                    auto& comp = deserializedEntity.GetComponent<PythonScriptComponent>();
                    auto behavioursNode = pythonScriptComponent["Behaviours"];
                    if (behavioursNode)
                    {
                        for (auto bindingNode : behavioursNode)
                        {
                            UUID classID;
                            if (bindingNode["ClassID"])
                                classID = (UUID)bindingNode["ClassID"].as<uint64_t>();
                            else if (bindingNode["ClassName"] && bindingNode["ModuleName"])
                            {
                                std::string className = bindingNode["ClassName"].as<std::string>();
                                std::string moduleName = bindingNode["ModuleName"].as<std::string>();
                                classID = PythonScriptMetaRegistry::GenerateClassID(moduleName + "." + className);
                            }

                            auto* ss = m_Scene->GetSystem<ScriptSystem>();
                            auto binding = ss->CreatePythonBinding(classID);
                            if (bindingNode["Enabled"])
                                binding.Enabled = bindingNode["Enabled"].as<bool>();

                            auto fieldsNode = bindingNode["Fields"];
                            if (fieldsNode)
                            {
                                for (auto fieldNode : fieldsNode)
                                {
                                    std::string fieldName = fieldNode["Name"].as<std::string>();
                                    ScriptFieldType fieldType = (ScriptFieldType)fieldNode["Type"].as<uint16_t>();
                                    uint32_t fieldHash = HashFieldName(fieldName);

                                    auto it = binding.Fields.find(fieldHash);
                                    if (it != binding.Fields.end() && it->second.GetType() == fieldType && fieldNode["Value"])
                                    {
                                        switch (fieldType)
                                        {
                                        case ScriptFieldType::Float:
                                            it->second.SetValue(fieldNode["Value"].as<float>());
                                            break;
                                        case ScriptFieldType::Double:
                                            it->second.SetValue(fieldNode["Value"].as<double>());
                                            break;
                                        case ScriptFieldType::Bool:
                                            it->second.SetValue(fieldNode["Value"].as<bool>());
                                            break;
                                        case ScriptFieldType::Int8:
                                            it->second.SetValue(fieldNode["Value"].as<int8_t>());
                                            break;
                                        case ScriptFieldType::Int16:
                                            it->second.SetValue(fieldNode["Value"].as<int16_t>());
                                            break;
                                        case ScriptFieldType::Int32:
                                            it->second.SetValue(fieldNode["Value"].as<int32_t>());
                                            break;
                                        case ScriptFieldType::Int64:
                                            it->second.SetValue(fieldNode["Value"].as<int64_t>());
                                            break;
                                        case ScriptFieldType::UInt8:
                                            it->second.SetValue(fieldNode["Value"].as<uint8_t>());
                                            break;
                                        case ScriptFieldType::UInt16:
                                            it->second.SetValue(fieldNode["Value"].as<uint16_t>());
                                            break;
                                        case ScriptFieldType::UInt32:
                                            it->second.SetValue(fieldNode["Value"].as<uint32_t>());
                                            break;
                                        case ScriptFieldType::UInt64:
                                            it->second.SetValue(fieldNode["Value"].as<uint64_t>());
                                            break;
                                        case ScriptFieldType::Vector2:
                                            it->second.SetValue(fieldNode["Value"].as<glm::vec2>());
                                            break;
                                        case ScriptFieldType::Vector3:
                                            it->second.SetValue(fieldNode["Value"].as<glm::vec3>());
                                            break;
                                        case ScriptFieldType::Vector4:
                                            it->second.SetValue(fieldNode["Value"].as<glm::vec4>());
                                            break;
                                        case ScriptFieldType::Object:
                                            break;
                                        }
                                    }
                                }
                            }

                            ss->RegisterPythonBinding(deserializedEntity, std::move(binding));
                        }
                    }

                    PR_CORE_INFO("  PythonScriptComponent: {0} Behaviours loaded", comp.Behaviours.size());
                }
            }
        }
        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
    {
        // Not implemented
        PR_CORE_ASSERT(false);
        return false;
    }

}
