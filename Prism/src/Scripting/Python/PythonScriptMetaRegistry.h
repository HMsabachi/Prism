#pragma once
#include "ScriptTypes.h"
#include "Scripting/Python/Interop/PythonScriptCore.h"

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

        static ScriptClassMetadata* GetClassMetadata(UUID classID);
        static ScriptClassMetadata* GetClassMetadata(const std::string& fullName);
        static std::vector<ScriptClassMetadata*> GetAllBehaviourClasses();

        static UUID GenerateClassID(const std::string& str);
    private:
        static void ScanDirectory(const std::string& dirPath, const std::string& packagePrefix,
                                  Python::ScriptClass& behaviourClass);
        static void ScanModule(Python::ScriptModule& mod, const std::string& moduleName,
                               Python::ScriptClass& behaviourClass);

        static void ReadPythonDefaultFieldValue(ScriptFieldMetadata& meta,
                                                Python::ScriptObject& obj,
                                                const std::string& fieldName);

        static std::unordered_map<UUID, ScriptClassMetadata> s_Classes;
        static std::unordered_map<UUID, std::string> s_ClassIDToFullName;
        static bool s_Initialized;
    };

}
