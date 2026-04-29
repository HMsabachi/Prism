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
}

namespace Prism
{
	enum class FieldType
	{
		None = 0, Float, Int, UnsignedInt, String, Vec2, Vec3, Vec4
	};

    struct PublicField
    {
        std::string Name;
		FieldType Type;
		Rolky::ManagedObject* m_Object;
    };

	using ScriptModuleFieldMap = std::unordered_map<std::string, std::vector<PublicField>>;
	class PRISM_API ScriptEngine
	{
	public:
		static bool Initialize();

		static void Shutdown();

		static bool LoadEngineAssembly(const std::string& assemblyPath);
		static bool LoadAppAssembly(const std::string& assemblyPath);

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(uint32_t entityID, float ts);

		static void OnInitEntity(ScriptComponent& script, uint32_t entityID, uint32_t sceneID);

		static const ScriptModuleFieldMap& GetFieldMap();

	public:
		static Rolky::ManagedAssembly& GetEngineAssembly();
	private:
		static std::unique_ptr<Rolky::HostInstance> m_Host;
		static std::unique_ptr<Rolky::AssemblyLoadContext> m_LoadContext;

		static void* s_HostHandle;
		static void* s_AssemblyLoadContext;   
	};
}