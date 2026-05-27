#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Core/UUID.h"
#include "Prism/Core/Buffer.h"
#include "ScriptTypes.h"

#include <Rolky/Assembly.hpp>
#include <Rolky/Type.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace Prism
{

    class PRISM_API CSharpScriptMetaRegistry
    {
    public:
        static void Init();
        static void Shutdown();

        static void BuildCache();

        static ScriptClassMetadata* GetClassMetadata(UUID classID);
        static ScriptClassMetadata* GetClassMetadata(const std::string& fullName);
        static std::vector<ScriptClassMetadata*> GetAllBehaviourClasses();

        static ScriptFieldMetadata* GetFieldMetadata(UUID classID, const std::string& fieldName);

        static UUID GenerateClassID(const std::string& str);
    private:
        static void BuildCacheForAssembly(Rolky::ManagedAssembly& assembly);
        static ScriptFieldType GetFieldTypeFromManagedType(Rolky::Type* type);

        static std::unordered_map<UUID, ScriptClassMetadata> s_Classes;
        static std::unordered_map<UUID, std::string> s_ClassIDToFullName;
        static Rolky::Type* s_BehaviourType;
        static bool s_Initialized;
    };

}
