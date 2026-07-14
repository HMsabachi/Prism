#include "prpch.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Core/Math/Noise.h"
#include "Prism/Core/Input.h"
#include "Prism/Physics/PXPhysicsWrappers.h"
#include "Prism/Physics/Physics.h"
#include "Prism/Physics/PhysicsActor.h"
#include "Prism/Physics/PhysicsUtil.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include "Prism/Scene/Systems/ScriptSystem.h"
#include "Prism/Scene/Systems/TransformSystem.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Asset/ModelImporter.h"
#include "PythonScriptWrappers.h"
#include "PythonScriptTypeCasters.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <glm/gtc/type_ptr.hpp>
#include <box2d/box2d.h>
#include <PhysX/PxPhysicsAPI.h>

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

        static TransformSystem* GetTransformSystem(Entity entity)
        {
            return entity.GetScene()->GetSystem<TransformSystem>();
        }
    }
} // namespace Prism::PythonScript

namespace Prism::PythonScript
{
    struct PythonTransform
    { glm::vec3 Position; glm::vec3 Rotation; glm::vec3 Scale; glm::vec3 Up; glm::vec3 Right; glm::vec3 Forward; };

    class PythonNoise
    {
    public:
        static float PerlinNoise(float x, float y) { return Noise::PerlinNoise(x, y); }
    };

    class PythonInput
    {
    public:
        static bool IsKeyPressed(KeyCode key) { return Input::IsKeyPressed(key); }
        static glm::vec2 GetMousePosition() { auto [x, y] = Input::GetMousePosition(); return { x, y }; }
        static void SetCursorMode(CursorMode mode) { Input::SetCursorMode(mode); }
        static CursorMode GetCursorMode() { return Input::GetCursorMode(); }
        static bool IsMouseButtonPressed(MouseButton button) { return Input::IsMouseButtonPressed(button); }
    };

    class PythonTime
    {
    public:
        static float GetDeltaTime(py::object) { return Time::GetDeltaTime(); }
        static float GetUnscaledDeltaTime(py::object) { return Time::GetUnscaledDeltaTime(); }
        static float GetTime(py::object) { return Time::GetTime(); }
        static float GetUnscaledTime(py::object) { return Time::GetUnscaledTime(); }
        static int64_t GetFrameCount(py::object) { return (int64_t)Time::GetFrameCount(); }
        static float GetFixedDeltaTime(py::object) { return Time::GetFixedDeltaTime(); }
        static void SetFixedDeltaTime(py::object, float fixedDeltaTime) { Time::SetFixedDeltaTime(fixedDeltaTime); }
        static float GetTimeScale(py::object) { return Time::GetTimeScale(); }
        static void SetTimeScale(py::object, float scale) { Time::SetTimeScale(scale); }
    };

    class PythonLog
    {
    public:
        static void Trace(const char* message) { PR_CORE_TRACE("[Python] {}", message); }
        static void Debug(const char* message) { PR_CORE_TRACE("[Python] {}", message); }
        static void Info(const char* message) { PR_CORE_INFO("[Python] {}", message); }
        static void Warn(const char* message) { PR_CORE_WARN("[Python] {}", message); }
        static void Error(const char* message) { PR_CORE_ERROR("[Python] {}", message); }
        static void Critical(const char* message) { PR_CORE_FATAL("[Python] {}", message); }
    };

    class PythonAsset
    {
    protected:
        uint64_t m_Handle = 0;
    public:
        PythonAsset(uint64_t handle) : m_Handle(handle) {}
        PythonAsset(const PythonAsset& other) : m_Handle(reinterpret_cast<uint64_t>(new Ref<Asset>(*reinterpret_cast<Ref<Asset>*>(other.m_Handle)))) {}
        PythonAsset& operator=(const PythonAsset& other)
        {
            if (m_Handle) delete (Ref<Asset>*)(m_Handle);
            m_Handle = reinterpret_cast<uint64_t>(new Ref<Asset>(*reinterpret_cast<Ref<Asset>*>(other.m_Handle)));
            return *this;
        }
        PythonAsset(PythonAsset&& other) noexcept : m_Handle(other.m_Handle) { other.m_Handle = 0; }
        PythonAsset& operator=(PythonAsset&& other) noexcept
        {
            if (m_Handle) delete (Ref<Asset>*)(m_Handle);
            m_Handle = other.m_Handle;
            other.m_Handle = 0;
            return *this;
        }
        virtual ~PythonAsset() { if (m_Handle) delete (Ref<Asset>*)(m_Handle); m_Handle = 0; }
        virtual std::string __Repr__() { return fmt::format(" <Asset Handle = {}>", m_Handle); }
        virtual uint64_t GetHandle() const { return m_Handle; }
    public:
        Ref<Asset>& GetInstance() const { return *reinterpret_cast<Ref<Asset>*>(m_Handle); }
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
    public:
        Ref<Mesh>& GetInstance() const { return *reinterpret_cast<Ref<Mesh>*>(m_Handle); }
    };

    class PythonTexture2D : public PythonAsset
    {
    public:
        PythonTexture2D(uint64_t handle) : PythonAsset(handle) {}
        PythonTexture2D(uint32_t width, uint32_t height) : PythonAsset(0) { m_Handle = reinterpret_cast<uint64_t>(new Ref<Texture2D>(Texture2D::Create(TextureFormat::RGBA, width, height))); }
        virtual ~PythonTexture2D() override { if (m_Handle) delete (Ref<Texture2D>*)(m_Handle); m_Handle = 0; }
        virtual std::string __Repr__() override
        {
            std::string result;
            auto& texture = GetInstance();
            if (texture)
            {
                result = fmt::format(" <Texture2D Handle = {} Width = {} height = {}>"
                    , m_Handle, texture->GetWidth(), texture->GetHeight());
            }
            else result = fmt::format(" <Texture2D Handle = {}>", m_Handle);
            return result;
        }
    public:
        Ref<Texture2D>& GetInstance() const { return *reinterpret_cast<Ref<Texture2D>*>(m_Handle); }
    };

    class PythonMaterial : public PythonAsset
    {
    public:
        PythonMaterial(uint64_t handle) : PythonAsset(handle) {}
        PythonMaterial(const char* shaderName) : PythonAsset(0)
        {
            const auto& shader = AssetManager::GetShaderLibrary()->Get(shaderName);
            m_Handle = reinterpret_cast<uint64_t>(new Ref<Material>(Material::Create(shader->Handle)));
        }
        virtual ~PythonMaterial() override { if (m_Handle) delete (Ref<Material>*)(m_Handle); m_Handle = 0; }
        virtual std::string __Repr__() override
        {
            std::string result;
            if (m_Handle)
            {
                auto& material = *reinterpret_cast<Ref<Material>*>(m_Handle);
                result = fmt::format(" <Material Handle = {} Shader = {}>"
                    , m_Handle, material->GetShader()->GetName());
            }
            else result = fmt::format(" <Material Handle = {}>", m_Handle);
            return result;
        }
        void SetFloat(const char* uniform, float value) { GetInstance()->SetFloat(uniform, value); }
        void SetInt(const char* uniform, int value) { GetInstance()->SetInt(uniform, value); }
        void SetBool(const char* uniform, bool value) { GetInstance()->SetBool(uniform, value); }
        void SetVector2(const char* uniform, const glm::vec2& value) { GetInstance()->SetVec2(uniform, value); }
        void SetVector3(const char* uniform, const glm::vec3& value) { GetInstance()->SetVec3(uniform, value); }
        void SetVector4(const char* uniform, const glm::vec4& value) { GetInstance()->SetVec4(uniform, value); }
        void SetColor3(const char* uniform, const glm::vec3& value) { GetInstance()->SetColor3(uniform, value); }
        void SetColor(const char* uniform, const glm::vec4& value) { GetInstance()->SetColor(uniform, value); }
        void SetMatrix4(const char* uniform, const glm::mat4& value) { GetInstance()->SetMatrix4(uniform, value); }
        void SetTexture(const char* uniform, const PythonTexture2D& texture) { GetInstance()->SetTexture(uniform, texture.GetInstance()); }
        void SetKeyword(const char* name, bool enabled) { GetInstance()->SetKeyword(name, enabled); }
        bool IsKeywordEnabled(const char* name) { return GetInstance()->IsKeywordEnabled(name); }
    public:
        Ref<Material>& GetInstance() const { return *reinterpret_cast<Ref<Material>*>(m_Handle); }
    };

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
                auto& comp = entity.GetComponent<Prism::PythonScriptComponent>();
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
                auto& comp = entity.GetComponent<Prism::PythonScriptComponent>();
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
        virtual ~PythonComponent() = default;
        virtual std::string __Repr__() { return fmt::format(" <Component EntityID = {}>", m_EntityID); }
        pybind11::object GetEntity() const { return m_Entity; }

        void SetEntity(pybind11::object entity)
        {
            m_Entity = entity;
            m_EntityID = entity.attr("ID").cast<uint64_t>();
        }
        
    protected:
        Entity GetEntityImpt() const
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            const auto& entityMap = scene->GetEntityMap();
            PR_CORE_ASSERT(entityMap.find(m_EntityID) != entityMap.end(),
                "Invalid entity ID or entity doesn't exist in scene!");
            return entityMap.at(m_EntityID);
        }
    };

    class PythonTagComponent : public PythonComponent
    {
        public:
        std::string GetTag() const { return GetEntityImpt().GetComponent<TagComponent>().Tag; }
        void SetTag(const std::string& tag) { GetEntityImpt().GetComponent<TagComponent>().Tag = tag; }
        virtual std::string __Repr__() override { return fmt::format(" <TagComponent Tag = {}>", GetTag()); }
    };

    class PythonScriptComponent : public PythonComponent
    {
    };

    class PythonCameraComponent : public PythonComponent
    {
    public:
        virtual std::string __Repr__() override { return fmt::format(" <CameraComponent> {}", m_EntityID); }
    };

    class PythonSpriteRendererComponent : public PythonComponent
    {
    };

    class PythonBoxCollider2DComponent : public PythonComponent
    {
    };

    class PythonCircleCollider2DComponent : public PythonComponent
    {
    };

    class PythonRigidBody2DComponent : public PythonComponent
    {
    public:
        virtual std::string __Repr__() override { return fmt::format(" <RigidBody2DComponent> {}", m_EntityID); }
        void ApplyLinearImpulse(const glm::vec2& impulse, const glm::vec2& offset, bool wake)
        {
            Entity entity = GetEntityImpt();
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), b2Vec2(offset.x, offset.y), wake);
        }
        glm::vec2 GetLinearVelocity() const
        {
            Entity entity = GetEntityImpt();
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            const b2Vec2& v = body->GetLinearVelocity();
            return { v.x, v.y };
        }
        void SetLinearVelocity(const glm::vec2& velocity)
        {
            Entity entity = GetEntityImpt();
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
        }
    };

    class PythonRigidBodyComponent : public PythonComponent
    {
    public:
        virtual std::string __Repr__() override { return fmt::format(" <RigidBodyComponent> {}", m_EntityID); }
        void AddForce(const glm::vec3& force, ForceMode mode)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
            actor->AddForce(force, (ForceMode)mode);
        }
        void AddTorque(const glm::vec3& torque, ForceMode mode)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
            actor->AddTorque(torque, (ForceMode)mode);
        }
        glm::vec3 GetLinearVelocity() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetLinearVelocity();
        }
        void SetLinearVelocity(const glm::vec3& velocity)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetLinearVelocity(velocity);
        }
        void Rotate(const glm::vec3& rotation)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->Rotate(rotation);
        }
        float GetMass() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetMass();
        }
        void SetMass(float mass)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetMass(mass);
        }
        glm::vec3 GetAngularVelocity() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetAngularVelocity();
        }
        void SetAngularVelocity(const glm::vec3& velocity)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetAngularVelocity(velocity);
        }
        uint32_t GetLayer() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return entity.GetComponent<RigidBodyComponent>().Layer;
        }
        uint32_t GetBodyType() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return (uint32_t)entity.GetComponent<RigidBodyComponent>().BodyType;
        }

    };

    class PythonBoxColliderComponent : public PythonComponent
    {
    };

    class PythonSphereColliderComponent : public PythonComponent
    {
    };

    class PythonCapsuleColliderComponent : public PythonComponent
    {
    };

    class PythonTransformComponent : public PythonComponent
    {
    public:
        glm::vec3 GetPosition() const { return GetEntityImpt().Transformation().GetPosition(); }
        void SetPosition(const glm::vec3& v) { GetEntityImpt().Transformation().SetPosition(v); }
        glm::vec3 GetRotation() const { return GetEntityImpt().Transformation().GetRotation(); }
        void SetRotation(const glm::vec3& v) { GetEntityImpt().Transformation().SetRotation(v); }
        glm::vec3 GetScale() const { return GetEntityImpt().Transformation().GetScale(); }
        void SetScale(const glm::vec3& v) { GetEntityImpt().Transformation().SetScale(v); }
        glm::vec3 GetLocalPosition() const { return GetEntityImpt().Transformation().GetPosition(); }
        void SetLocalPosition(const glm::vec3& v) { GetEntityImpt().Transformation().SetPosition(v); }
        glm::vec3 GetLocalRotation() const { return GetEntityImpt().Transformation().GetRotation(); }
        void SetLocalRotation(const glm::vec3& v) { GetEntityImpt().Transformation().SetRotation(v); }
        glm::vec3 GetLocalScale() const { return GetEntityImpt().Transformation().GetScale(); }
        void SetLocalScale(const glm::vec3& v) { GetEntityImpt().Transformation().SetScale(v); }

        glm::vec3 GetForward() const { return GetEntityImpt().Transformation().Forward; }
        glm::vec3 GetRight() const { return GetEntityImpt().Transformation().Right; }
        glm::vec3 GetUp() const { return GetEntityImpt().Transformation().Up; }

        PythonTransform GetTransform() const {
            Entity e = GetEntityImpt();
            auto world = GetTransformSystem(e)->GetWorldDecomposed(e);
            auto& tc = e.Transformation();
            return { world.Position, world.Rotation, world.Scale, tc.Up, tc.Right, tc.Forward };
        }
        void SetTransform(const PythonTransform& t) {
            Entity e = GetEntityImpt();
            auto* ts = GetTransformSystem(e);
            ts->SetWorldPosition(e, t.Position);
            ts->SetWorldRotation(e, t.Rotation);
            ts->SetWorldScale(e, t.Scale);
        }
    };

    class PythonMeshRendererComponent : public PythonComponent
    {
    public:
        PythonMesh GetMesh() const
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            if (mc.Mesh)
                return PythonMesh(reinterpret_cast<uint64_t>(new Ref<Mesh>(mc.Mesh)));
            return PythonMesh((uint64_t)0);
        }
        void SetMesh(const PythonMesh& mesh)
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            if (mesh.GetHandle())
                mc.Mesh = *reinterpret_cast<Ref<Mesh>*>(mesh.GetHandle());
            else
                mc.Mesh = nullptr;
        }
        PythonMaterial GetMaterial(uint32_t index = 0) const
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            if (!mc.Materials.empty() && mc.Materials[index])
                return PythonMaterial(reinterpret_cast<uint64_t>(new Ref<Material>(mc.Materials[index])));
            return PythonMaterial((uint64_t)0);
        }
        void SetMaterial(const PythonMaterial& material, uint32_t index = 0)
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            if (!mc.Materials.empty())
            {
                if (material.GetHandle())
                    mc.Materials[index] = *reinterpret_cast<Ref<Material>*>(material.GetHandle());
                else PR_CORE_WARN("[Python] Attempted to set null material on MeshRendererComponent!");
            }
        }
        std::vector<PythonMaterial> GetMaterials() const
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            std::vector<PythonMaterial> result;
            result.reserve(mc.Materials.size());
            for (auto& mat : mc.Materials)
            {
                if (mat)
                    result.emplace_back(reinterpret_cast<uint64_t>(new Ref<Material>(mat)));
                else
                    result.emplace_back(static_cast<uint64_t>(0));
            }
            return result;
        }
        void SetMaterials(const std::vector<PythonMaterial>& materials)
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            mc.Materials.resize(materials.size());
            for (size_t i = 0; i < materials.size(); ++i)
            {
                if (materials[i].GetHandle())
                    mc.Materials[i] = *reinterpret_cast<Ref<Material>*>(materials[i].GetHandle());
                else
                    mc.Materials[i] = nullptr;
            }
        }
        uint32_t GetMaterialCount() const
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            return (uint32_t)mc.Materials.size();
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

    class PythonCollider
    {
    private:
        PythonEntity m_Entity;
        bool m_IsTrigger = false;
    public:
        PythonCollider(const PythonEntity& entity, bool isTrigger)
            : m_Entity(entity), m_IsTrigger(isTrigger) {}
        virtual ~PythonCollider() = default;
        virtual std::string __Repr__() { return fmt::format(" <Collider>"); }
        virtual std::string __Str__() { return fmt::format("Collider({}, {}, {})", GetColliderType(), m_Entity.GetID(), m_IsTrigger); }
        PythonEntity GetEntity() const { return m_Entity; }
        PythonRigidBodyComponent GetRigidBody() const
        {
            Entity entity = GetEntityImpt();
            if (entity.HasComponent<RigidBodyComponent>())
            {
                PythonRigidBodyComponent rb;
                rb.SetEntity(py::cast(m_Entity));
                return rb;
            }
            throw std::runtime_error("Entity does not have a RigidBodyComponent!");
        }
        bool IsTrigger() const { return m_IsTrigger; }
    protected:
        Entity GetEntityImpt() const
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            const auto& entityMap = scene->GetEntityMap();
            PR_CORE_ASSERT(entityMap.find(m_Entity.GetID()) != entityMap.end(),
                "Invalid entity ID or entity doesn't exist in scene!");
            return entityMap.at(m_Entity.GetID());
        }
        virtual std::string GetColliderType() const { return "Collder"; }
    };

    class PythonBoxCollider : public PythonCollider
    {
    private:
        glm::vec3 m_Size;
        glm::vec3 m_Offset;
    public:
        PythonBoxCollider(const PythonEntity& entity, bool isTrigger, const glm::vec3& size, const glm::vec3& offset)
            : PythonCollider(entity, isTrigger), m_Size(size), m_Offset(offset) {}
        glm::vec3 GetSize() const { return m_Size; }
        glm::vec3 GetOffset() const { return m_Offset; }
    protected:
        virtual std::string GetColliderType() const override { return "BoxCollider"; }
    };

    class PythonSphereCollider : public PythonCollider
    {
    private:
        float m_Radius;
    public:
        PythonSphereCollider(const PythonEntity& entity, bool isTrigger, float radius)
            : PythonCollider(entity, isTrigger), m_Radius(radius) {}
        float GetRadius() const { return m_Radius; }
    protected:
        virtual std::string GetColliderType() const override { return "SphereCollider"; }
    };

    class PythonCapsuleCollider : public PythonCollider
    {
    private:
        float m_Radius;
        float m_Height;
    public:
        PythonCapsuleCollider(const PythonEntity& entity, bool isTrigger, float radius, float height)
            : PythonCollider(entity, isTrigger), m_Radius(radius), m_Height(height) {}
        float GetRadius() const { return m_Radius; }
        float GetHeight() const { return m_Height; }
    protected:
        virtual std::string GetColliderType() const override { return "CapsuleCollider"; }
    };

    class PythonMeshCollider : public PythonCollider
    {
    private:
        PythonMesh m_Mesh;
    public:
        PythonMeshCollider(const PythonEntity& entity, bool isTrigger, const PythonMesh& mesh )
            : PythonCollider(entity, isTrigger), m_Mesh(mesh) {}
        PythonMesh GetMesh() const { return m_Mesh; }
    protected:
        virtual std::string GetColliderType() const override { return "MeshCollider"; }
    };

    class PythonPhysics
    {
        using ColliderList = std::vector<std::shared_ptr<PythonCollider>>;
    public:
        static float GetGravity() { return Physics::GetGravity(); }
        static void SetGravity(float gravity) { Physics::SetGravity(gravity); }
        static bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* raycastHit)
        {
            RaycastHit hit;
            if (PXPhysicsWrappers::Raycast(origin, direction, maxDistance, &hit))
            {
                if (raycastHit) *raycastHit = hit;
                return true;
            }
            return false;
        }
        static ColliderList OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            ColliderList results;
            if (PXPhysicsWrappers::OverlapBox(origin, halfSize, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }
        static ColliderList OverlapSphere(const glm::vec3& origin, float radius)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            ColliderList results;
            if (PXPhysicsWrappers::OverlapSphere(origin, radius, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }
        static ColliderList OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            ColliderList results;
            if (PXPhysicsWrappers::OverlapCapsule(origin, radius, halfHeight, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }
    private:
        static std::shared_ptr<PythonCollider> FillOverlapHit(physx::PxOverlapHit& pxHit)
        {
            Entity& entity = *(Entity*)pxHit.actor->userData;
            if (entity.HasComponent<BoxColliderComponent>())
            {
                auto& bc = entity.GetComponent<BoxColliderComponent>();
                auto boxCollider = std::make_shared<PythonBoxCollider>(
                    PythonEntity(entity.GetUUID()), bc.IsTrigger, bc.Size, bc.Offset
                );
                return boxCollider;
            }
            else if (entity.HasComponent<SphereColliderComponent>())
            {
                auto& sc = entity.GetComponent<SphereColliderComponent>();
                auto sphereCollider = std::make_shared<PythonSphereCollider>(
                    PythonEntity(entity.GetUUID()), sc.IsTrigger, sc.Radius
                );
                return sphereCollider;
            }
            else if (entity.HasComponent<CapsuleColliderComponent>())
            {
                auto& cc = entity.GetComponent<CapsuleColliderComponent>();
                auto capsuleCollider = std::make_shared<PythonCapsuleCollider>(
                    PythonEntity(entity.GetUUID()), cc.IsTrigger, cc.Radius, cc.Height
                );
                return capsuleCollider;
            }
            else if (entity.HasComponent<MeshColliderComponent>())
            {
                auto& mc = entity.GetComponent<MeshColliderComponent>();
                auto meshCollider = std::make_shared<PythonMeshCollider>(
                    PythonEntity(entity.GetUUID()), mc.IsTrigger, PythonMesh(reinterpret_cast<uint64_t>(new Ref<Mesh>(mc.CollisionMesh)))
                );
                return meshCollider;
            }
            return nullptr;
        }
    };

    class PythonMeshFactory
    {
    public:
        static PythonMesh CreatePlane(float width, float height)
        {
            return reinterpret_cast<uint64_t>(new Ref<Mesh>(ModelImporter::Import("assets/models/Plane1m.obj").Mesh));
        }
    };
} // namespace Prism::PythonScript

// PrismEngine Module Registe

PYBIND11_MODULE(PrismEngine, m)
{
    using namespace Prism::PythonScript;

    py::class_<PythonNoise>(m, "Noise")
        .def_static("PerlinNoise", &PythonNoise::PerlinNoise);

    py::class_<PythonInput>(m, "Input")
        .def_static("IsKeyPressed", &PythonInput::IsKeyPressed)
        .def_static("GetMousePosition", &PythonInput::GetMousePosition)
        .def_static("SetCursorMode", &PythonInput::SetCursorMode)
        .def_static("GetCursorMode", &PythonInput::GetCursorMode)
        .def_static("IsMouseButtonPressed", &PythonInput::IsMouseButtonPressed);

    py::class_<PythonTime>(m, "Time")
        .def_property_readonly_static("DeltaTime", &PythonTime::GetDeltaTime)
        .def_property_readonly_static("UnscaledDeltaTime", &PythonTime::GetUnscaledDeltaTime)
        .def_property_readonly_static("Time", &PythonTime::GetTime)
        .def_property_readonly_static("UnscaledTime", &PythonTime::GetUnscaledTime)
        .def_property_readonly_static("FrameCount", &PythonTime::GetFrameCount)
        .def_property_static("TimeScale", &PythonTime::GetTimeScale, &PythonTime::SetTimeScale)
        .def_property_static("FixedDeltaTime", &PythonTime::GetFixedDeltaTime, &PythonTime::SetFixedDeltaTime);

    py::class_<PythonLog>(m, "Log")
        .def_static("Trace", &PythonLog::Trace)
        .def_static("Debug", &PythonLog::Debug)
        .def_static("Info", &PythonLog::Info)
        .def_static("Warn", &PythonLog::Warn)
        .def_static("Error", &PythonLog::Error)
        .def_static("Critical", &PythonLog::Critical);

    py::class_<PythonAsset>(m, "Asset")
        .def(py::init<uint64_t>())
        .def("__repr__", &PythonAsset::__Repr__)
        .def_property_readonly("_handle", &PythonMesh::GetHandle); // TODO: Remove this
    py::class_<PythonMesh, PythonAsset>(m, "Mesh")
        .def(py::init<uint64_t>())
        .def(py::init<const char*>())
        .def("__repr__", &PythonMesh::__Repr__);
    py::class_<PythonTexture2D, PythonAsset>(m, "Texture2D")
        .def(py::init<uint64_t>())
        .def(py::init<uint32_t, uint32_t>())
        .def("__repr__", &PythonTexture2D::__Repr__);
    py::class_<PythonMaterial, PythonAsset>(m, "Material")
        .def(py::init<uint64_t>())
        .def(py::init<const char*>())
        .def("__repr__", &PythonMaterial::__Repr__)
        .def("SetFloat", &PythonMaterial::SetFloat)
        .def("SetInt", &PythonMaterial::SetInt)
        .def("SetBool", &PythonMaterial::SetBool)
        .def("SetVector2", &PythonMaterial::SetVector2)
        .def("SetVector3", &PythonMaterial::SetVector3)
        .def("SetVector4", &PythonMaterial::SetVector4)
        .def("SetColor3", &PythonMaterial::SetColor3)
        .def("SetColor", &PythonMaterial::SetColor)
        .def("SetMatrix4", &PythonMaterial::SetMatrix4)
        .def("SetTexture", &PythonMaterial::SetTexture)
        .def("SetKeyword", &PythonMaterial::SetKeyword)
        .def("IsKeywordEnabled", &PythonMaterial::IsKeywordEnabled);
    py::class_<PythonMeshFactory>(m, "MeshFactory")
        .def_static("CreatePlane", &PythonMeshFactory::CreatePlane);

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
    py::class_<PythonTagComponent, PythonComponent>(m, "TagComponent")
        .def(py::init<>())
        .def("__repr__", &PythonTagComponent::__Repr__)
        .def_property("Tag", &PythonTagComponent::GetTag, &PythonTagComponent::SetTag);
    py::class_<PythonCameraComponent, PythonComponent>(m, "CameraComponent")
        .def(py::init<>())
        .def("__repr__", &PythonCameraComponent::__Repr__);
    py::class_<::Prism::PythonScript::PythonScriptComponent, PythonComponent>(m, "ScriptComponent")
        .def(py::init<>());
    py::class_<PythonSpriteRendererComponent, PythonComponent>(m, "SpriteRendererComponent")
        .def(py::init<>());
    py::class_<PythonRigidBody2DComponent, PythonComponent>(m, "RigidBody2DComponent")
        .def(py::init<>())
        .def("__repr__", &PythonRigidBody2DComponent::__Repr__)
        .def("ApplyLinearImpulse", &PythonRigidBody2DComponent::ApplyLinearImpulse)
        .def("GetLinearVelocity", &PythonRigidBody2DComponent::GetLinearVelocity)
        .def("SetLinearVelocity", &PythonRigidBody2DComponent::SetLinearVelocity)
        .def_property("LinearVelocity", &PythonRigidBody2DComponent::GetLinearVelocity, &PythonRigidBody2DComponent::SetLinearVelocity);
    py::class_<PythonBoxCollider2DComponent, PythonComponent>(m, "BoxCollider2DComponent")
        .def(py::init<>());
    py::class_<PythonCircleCollider2DComponent, PythonComponent>(m, "CircleCollider2DComponent")
        .def(py::init<>());

    py::class_<PythonRigidBodyComponent, PythonComponent>(m, "RigidBodyComponent")
        .def(py::init<>())
        .def("__repr__", &PythonRigidBodyComponent::__Repr__)
        .def("AddForce", &PythonRigidBodyComponent::AddForce)
        .def("AddTorque", &PythonRigidBodyComponent::AddTorque)
        .def_property("LinearVelocity", &PythonRigidBodyComponent::GetLinearVelocity, &PythonRigidBodyComponent::SetLinearVelocity)
        .def("GetLinearVelocity", &PythonRigidBodyComponent::GetLinearVelocity)
        .def("SetLinearVelocity", &PythonRigidBodyComponent::SetLinearVelocity)
        .def("Rotate", &PythonRigidBodyComponent::Rotate)
        .def_property("Mass", &PythonRigidBodyComponent::GetMass, &PythonRigidBodyComponent::SetMass)
        .def("GetMass", &PythonRigidBodyComponent::GetMass)
        .def("SetMass", &PythonRigidBodyComponent::SetMass)
        .def_property("AngularVelocity", &PythonRigidBodyComponent::GetAngularVelocity, &PythonRigidBodyComponent::SetAngularVelocity)
        .def("GetAngularVelocity", &PythonRigidBodyComponent::GetAngularVelocity)
        .def("SetAngularVelocity", &PythonRigidBodyComponent::SetAngularVelocity)
        .def_property_readonly("Layer", &PythonRigidBodyComponent::GetLayer)
        .def_property_readonly("BodyType", &PythonRigidBodyComponent::GetBodyType);

    py::class_<PythonBoxColliderComponent, PythonComponent>(m, "BoxColliderComponent")
        .def(py::init<>());
    py::class_<PythonSphereColliderComponent, PythonComponent>(m, "SphereColliderComponent")
        .def(py::init<>());
    py::class_<PythonCapsuleColliderComponent, PythonComponent>(m, "CapsuleColliderComponent")
        .def(py::init<>());

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
    py::class_<PythonMeshRendererComponent, PythonComponent>(m, "MeshRendererComponent")
        .def(py::init<>())
        .def_property("Mesh", &PythonMeshRendererComponent::GetMesh, &PythonMeshRendererComponent::SetMesh)
        .def_property("Material",
            [](const PythonMeshRendererComponent& self) { return self.GetMaterial(0); },
            [](PythonMeshRendererComponent& self, const PythonMaterial& mat) { self.SetMaterial(mat, 0); })
        .def_property("Materials", &PythonMeshRendererComponent::GetMaterials, &PythonMeshRendererComponent::SetMaterials)
        .def_property_readonly("MaterialCount", &PythonMeshRendererComponent::GetMaterialCount)
        .def("GetMaterial", &PythonMeshRendererComponent::GetMaterial)
        .def("SetMaterial", &PythonMeshRendererComponent::SetMaterial);
    py::class_<PythonBehaviour, PythonComponent>(m, "Behaviour")
        .def(py::init<>())
        .def_property("ID", &PythonBehaviour::GetID, &PythonBehaviour::SetID)
        .def_property("Enabled", &PythonBehaviour::GetEnabled, &PythonBehaviour::SetEnabled)
        .def_property_readonly("Transform", &PythonBehaviour::GetTransform)
        .def("GetComponent", &PythonBehaviour::GetComponent)
        .def("HasComponent", &PythonBehaviour::HasComponent)
        .def("CreateComponent", &PythonBehaviour::CreateComponent);

    py::class_<PythonCollider>(m, "Collider")
        .def("__repr__", &PythonCollider::__Repr__)
        .def("__str__", &PythonCollider::__Str__)
        .def_property_readonly("Entity", &PythonCollider::GetEntity)
        .def_property_readonly("RigidBody", &PythonCollider::GetRigidBody)
        .def_property_readonly("IsTrigger", &PythonCollider::IsTrigger);
    py::class_<PythonBoxCollider, PythonCollider>(m, "BoxCollider")
        .def_property_readonly("Size", &PythonBoxCollider::GetSize)
        .def_property_readonly("Offset", &PythonBoxCollider::GetOffset);
    py::class_<PythonSphereCollider, PythonCollider>(m, "SphereCollider")
        .def_property_readonly("Radius", &PythonSphereCollider::GetRadius);
    py::class_<PythonCapsuleCollider, PythonCollider>(m, "CapsuleCollider")
        .def_property_readonly("Radius", &PythonCapsuleCollider::GetRadius)
        .def_property_readonly("Height", &PythonCapsuleCollider::GetHeight);
    py::class_<PythonMeshCollider, PythonCollider>(m, "MeshCollider")
        .def_property_readonly("Mesh", &PythonMeshCollider::GetMesh);

    py::class_<PythonPhysics>(m, "Physics")
        .def_property_static("Gravity", &PythonPhysics::GetGravity, &PythonPhysics::SetGravity)
        .def_static("Raycast", &PythonPhysics::Raycast,
            py::arg("origin"), py::arg("direction"), py::arg("maxDistance") = 100.0f, py::arg("raycastHit") = py::none())
        .def_static("OverlapBox", &PythonPhysics::OverlapBox)
        .def_static("OverlapSphere", &PythonPhysics::OverlapSphere)
        .def_static("OverlapCapsule", &PythonPhysics::OverlapCapsule);


#pragma region EnumClass
    py::class_<RaycastHit>(m, "RaycastHit")
        .def(py::init<>())
        .def_readwrite("EntityID", &RaycastHit::EntityID)
        .def_readwrite("Position", &RaycastHit::Position)
        .def_readwrite("Normal", &RaycastHit::Normal)
        .def_readwrite("Distance", &RaycastHit::Distance);

    py::class_<PythonTransform>(m, "ScriptTransform")
        .def(py::init<>())
        .def_readwrite("Position", &PythonTransform::Position)
        .def_readwrite("Rotation", &PythonTransform::Rotation)
        .def_readwrite("Scale", &PythonTransform::Scale)
        .def_readwrite("Up", &PythonTransform::Up)
        .def_readwrite("Right", &PythonTransform::Right)
        .def_readwrite("Forward", &PythonTransform::Forward);

    py::enum_<ForceMode>(m, "ForceMode")
        .value("Force", ForceMode::Force)
        .value("Impulse", ForceMode::Impulse)
        .value("VelocityChange", ForceMode::VelocityChange)
        .value("Acceleration", ForceMode::Acceleration);
    py::enum_<CursorMode>(m, "CursorMode")
        .value("Normal", CursorMode::Normal)
        .value("Hidden", CursorMode::Hidden)
        .value("Locked", CursorMode::Locked);
    py::enum_<MouseButton>(m, "MouseButton")
        .value("Left", MouseButton::Left)
        .value("Right", MouseButton::Right)
        .value("Middle", MouseButton::Middle)
        .value("Button0", MouseButton::Button0)
        .value("Button1", MouseButton::Button1)
        .value("Button2", MouseButton::Button2)
        .value("Button3", MouseButton::Button3)
        .value("Button4", MouseButton::Button4)
        .value("Button5", MouseButton::Button5);
    py::enum_<KeyCode>(m, "KeyCode")
        .value("Space", KeyCode::Space)
        .value("Apostrophe", KeyCode::Apostrophe)
        .value("Comma", KeyCode::Comma)
        .value("Minus", KeyCode::Minus)
        .value("Period", KeyCode::Period)
        .value("Slash", KeyCode::Slash)
        .value("D0", KeyCode::D0)
        .value("D1", KeyCode::D1)
        .value("D2", KeyCode::D2)
        .value("D3", KeyCode::D3)
        .value("D4", KeyCode::D4)
        .value("D5", KeyCode::D5)
        .value("D6", KeyCode::D6)
        .value("D7", KeyCode::D7)
        .value("D8", KeyCode::D8)
        .value("D9", KeyCode::D9)
        .value("Semicolon", KeyCode::Semicolon)
        .value("Equal", KeyCode::Equal)
        .value("A", KeyCode::A)
        .value("B", KeyCode::B)
        .value("C", KeyCode::C)
        .value("D", KeyCode::D)
        .value("E", KeyCode::E)
        .value("F", KeyCode::F)
        .value("G", KeyCode::G)
        .value("H", KeyCode::H)
        .value("I", KeyCode::I)
        .value("J", KeyCode::J)
        .value("K", KeyCode::K)
        .value("L", KeyCode::L)
        .value("M", KeyCode::M)
        .value("N", KeyCode::N)
        .value("O", KeyCode::O)
        .value("P", KeyCode::P)
        .value("Q", KeyCode::Q)
        .value("R", KeyCode::R)
        .value("S", KeyCode::S)
        .value("T", KeyCode::T)
        .value("U", KeyCode::U)
        .value("V", KeyCode::V)
        .value("W", KeyCode::W)
        .value("X", KeyCode::X)
        .value("Y", KeyCode::Y)
        .value("Z", KeyCode::Z)
        .value("LeftBracket", KeyCode::LeftBracket)
        .value("Backslash", KeyCode::Backslash)
        .value("RightBracket", KeyCode::RightBracket)
        .value("GraveAccent", KeyCode::GraveAccent)
        .value("World1", KeyCode::World1)
        .value("World2", KeyCode::World2)
        .value("Escape", KeyCode::Escape)
        .value("Enter", KeyCode::Enter)
        .value("Tab", KeyCode::Tab)
        .value("Backspace", KeyCode::Backspace)
        .value("Insert", KeyCode::Insert)
        .value("Delete", KeyCode::Delete)
        .value("Right", KeyCode::Right)
        .value("Left", KeyCode::Left)
        .value("Down", KeyCode::Down)
        .value("Up", KeyCode::Up)
        .value("PageUp", KeyCode::PageUp)
        .value("PageDown", KeyCode::PageDown)
        .value("Home", KeyCode::Home)
        .value("End", KeyCode::End)
        .value("CapsLock", KeyCode::CapsLock)
        .value("ScrollLock", KeyCode::ScrollLock)
        .value("NumLock", KeyCode::NumLock)
        .value("PrintScreen", KeyCode::PrintScreen)
        .value("Pause", KeyCode::Pause)
        .value("F1", KeyCode::F1)
        .value("F2", KeyCode::F2)
        .value("F3", KeyCode::F3)
        .value("F4", KeyCode::F4)
        .value("F5", KeyCode::F5)
        .value("F6", KeyCode::F6)
        .value("F7", KeyCode::F7)
        .value("F8", KeyCode::F8)
        .value("F9", KeyCode::F9)
        .value("F10", KeyCode::F10)
        .value("F11", KeyCode::F11)
        .value("F12", KeyCode::F12)
        .value("F13", KeyCode::F13)
        .value("F14", KeyCode::F14)
        .value("F15", KeyCode::F15)
        .value("F16", KeyCode::F16)
        .value("F17", KeyCode::F17)
        .value("F18", KeyCode::F18)
        .value("F19", KeyCode::F19)
        .value("F20", KeyCode::F20)
        .value("F21", KeyCode::F21)
        .value("F22", KeyCode::F22)
        .value("F23", KeyCode::F23)
        .value("F24", KeyCode::F24)
        .value("F25", KeyCode::F25)
        .value("KP0", KeyCode::KP0)
        .value("KP1", KeyCode::KP1)
        .value("KP2", KeyCode::KP2)
        .value("KP3", KeyCode::KP3)
        .value("KP4", KeyCode::KP4)
        .value("KP5", KeyCode::KP5)
        .value("KP6", KeyCode::KP6)
        .value("KP7", KeyCode::KP7)
        .value("KP8", KeyCode::KP8)
        .value("KP9", KeyCode::KP9)
        .value("KPDecimal", KeyCode::KPDecimal)
        .value("KPDivide", KeyCode::KPDivide)
        .value("KPMultiply", KeyCode::KPMultiply)
        .value("KPSubtract", KeyCode::KPSubtract)
        .value("KPAdd", KeyCode::KPAdd)
        .value("KPEnter", KeyCode::KPEnter)
        .value("KPEqual", KeyCode::KPEqual)
        .value("LeftShift", KeyCode::LeftShift)
        .value("LeftControl", KeyCode::LeftControl)
        .value("LeftAlt", KeyCode::LeftAlt)
        .value("LeftSuper", KeyCode::LeftSuper)
        .value("RightShift", KeyCode::RightShift)
        .value("RightControl", KeyCode::RightControl)
        .value("RightAlt", KeyCode::RightAlt)
        .value("RightSuper", KeyCode::RightSuper)
        .value("Menu", KeyCode::Menu);
        
#pragma endregion

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
            s_PythonTypeCache[PYTHON_TYPE_MATERIALREF] = py::type::of<PythonMaterial>();
            s_PythonTypeCache[PYTHON_TYPE_TEXTURE2DREF] = py::type::of<PythonTexture2D>();
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
