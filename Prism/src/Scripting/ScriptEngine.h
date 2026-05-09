#pragma once
#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"

namespace Rolky
{
    struct HostInstance;
    struct AssemblyLoadContext;
    struct ManagedAssembly;
    class FieldInfo;
    class ManagedObject;
    class Type;
}

namespace Prism
{
    enum class FieldType
    {
        None = 0, Float, Int, UnsignedInt, String, Vec2, Vec3, Vec4
    };

    struct EntityInstance
    {
        Rolky::Type* ScriptClass = nullptr;
        std::unique_ptr<Rolky::ManagedObject> Object;
        Scene* SceneInstance = nullptr;
        uint32_t HasMethods;
    };

    struct PublicField
    {
        std::string Name;
        FieldType Type;
        PublicField(const std::string& name, FieldType type);
        PublicField(const PublicField&) = delete;
        PublicField(PublicField&& other);
        ~PublicField();

        void CopyStoredValueToRuntime();
        bool IsRuntimeAvailable() const;

        template<typename T>
        T GetStoredValue() const
        {
            T value;
            GetStoredValue_Internal(&value);
            return value;
        }

        template<typename T>
        void SetStoredValue(T value) const
        {
            SetStoredValue_Internal(&value);
        }

        template<typename T>
        T GetRuntimeValue() const
        {
            T value;
            GetRuntimeValue_Internal(&value);
            return value;
        }

        template<typename T>
        void SetRuntimeValue(T value) const
        {
            SetRuntimeValue_Internal(&value);
        }

        void SetStoredValueRaw(void* src);
    private:
        EntityInstance* m_EntityInstance;
        uint8_t* m_StoredValueBuffer = nullptr;

        uint8_t* AllocateBuffer(FieldType type);
        void SetStoredValue_Internal(void* value) const;
        void GetStoredValue_Internal(void* outValue) const;
        void SetRuntimeValue_Internal(void* value) const;
        void GetRuntimeValue_Internal(void* outValue) const;

        friend class ScriptEngine;
    };

    using ScriptModuleFieldMap = std::unordered_map<std::string, std::unordered_map<std::string, PublicField>>;

    struct EntityInstanceData
    {
        EntityInstance Instance;
        ScriptModuleFieldMap ModuleFieldMap;
    };

    using EntityInstanceMap = std::unordered_map<UUID, std::unordered_map<UUID, EntityInstanceData>>;
    class PRISM_API ScriptEngine
    {
    public:
        static bool Initialize();

        static void Shutdown();

        static bool LoadEngineAssembly(const std::string& assemblyPath);
        static bool LoadAppAssembly(const std::string& assemblyPath);

        static void OnSceneDestruct(UUID sceneID);
        static void ReloadAssembly(const std::string& path);

        static void SetSceneContext(const Ref<Scene>& scene);
        static const Ref<Scene>& GetCurrentSceneContext();

        static void CopyEntityScriptData(UUID dst, UUID src);

        static void OnCreateEntity(Entity entity);
        static void OnCreateEntity(UUID sceneID, UUID entityID);
        static void OnUpdateEntity(UUID sceneID, UUID entityID, float ts);
        static void OnFixedUpdateEntity(UUID sceneID, UUID entityID);

        static void OnCollision2DBegin(Entity entity);
        static void OnCollision2DBegin(UUID sceneID, UUID entityID);
        static void OnCollision2DEnd(Entity entity);
        static void OnCollision2DEnd(UUID sceneID, UUID entityID);

        static void OnCollisionBegin(Entity entity);
        static void OnCollisionBegin(UUID sceneID, UUID entityID);
        static void OnCollisionEnd(Entity entity);
        static void OnCollisionEnd(UUID sceneID, UUID entityID);

        static void OnScriptComponentDestroyed(UUID sceneID, UUID entityID);

        static bool ModuleExists(const std::string& moduleName);
        static void InitScriptEntity(Entity entity);
        static void ShutdownScriptEntity(Entity entity, const std::string& moduleName);
        static void InstantiateEntityClass(Entity entity);

        static EntityInstanceMap& GetEntityInstanceMap();
        static EntityInstanceData& GetEntityInstanceData(UUID sceneID, UUID entityID);

        // Debug
        static void OnImGuiRender();
    public:
        static Rolky::ManagedAssembly& GetEngineAssembly();
    private:
        static std::unique_ptr<Rolky::HostInstance> m_Host;
        static std::unique_ptr<Rolky::AssemblyLoadContext> m_LoadContext;

        static void* s_HostHandle;
        static void* s_AssemblyLoadContext;   
    };
}