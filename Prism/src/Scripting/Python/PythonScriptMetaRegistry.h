#pragma once
#include "ScriptTypes.h"
#include "PythonScriptCore.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Prism
{

    class PRISM_API PythonScriptMetaRegistry
    {
    public:
        static void Init();
        static void Shutdown();

        static void BuildCache();

        static ScriptClassMetadata* GetClassMetadata(UUID scriptID);
        static ScriptClassMetadata* GetClassMetadata(const std::string& fullName);
        static std::vector<ScriptClassMetadata*> GetAllBehaviourClasses();

    private:
        static void ScanDirectory(const std::string& dirPath, const std::string& packagePrefix,
                                  Python::ScriptClass& behaviourClass);
        static void ScanModule(Python::ScriptModule& mod, const std::string& moduleName,
                               Python::ScriptClass& behaviourClass);

        static UUID GenerateScriptID(const std::string& str);
        static void ReadPythonDefaultFieldValue(ScriptFieldMetadata& meta,
                                                Python::ScriptObject& obj,
                                                const std::string& fieldName);

        static std::unordered_map<UUID, ScriptClassMetadata> s_Classes;
        static std::unordered_map<std::string, UUID> s_FullNameToID;
        static bool s_Initialized;
    };

}
