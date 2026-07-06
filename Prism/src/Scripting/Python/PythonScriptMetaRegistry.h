#pragma once
#include "ScriptTypes.h"
#include <string>
#include <vector>

namespace pybind11
{
    class module_;
    class object;
}

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
        static ScriptFieldMetadata* GetFieldMetadata(UUID classID, const std::string& fieldName);
        static std::vector<ScriptClassMetadata*> GetAllBehaviourClasses();

        static UUID GenerateClassID(const std::string& str);

    private:
        static bool s_Initialized;
        static std::unordered_map<UUID, ScriptClassMetadata> s_Classes;
        static std::unordered_map<UUID, std::string> s_ClassIDToFullName;

        static void ScanModule(pybind11::module_& mod, const std::string& moduleName, pybind11::object& behaviourClass);
        static void ScanDirectory(const std::string& dirPath, const std::string& packagePrefix, pybind11::object& behaviourClass);
    };

}
