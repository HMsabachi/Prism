#include "Rolky/Builder.hpp"
#include "Rolky/String.hpp"

#include "RolkyManagedFunctions.hpp"

namespace Rolky {

    void BuildManager::SetLogsDirectory(std::string_view logsDirectory)
    {
        m_LogsDirectory = logsDirectory;
    }

    bool BuildManager::Build(ScriptSolution& solution, std::string_view configuration, bool rebuild)
    {
        solution.Generate();

        ScopedString sln  = String::New(solution.GetSolutionPath());
        ScopedString proj = String::New(solution.GetSolutionPath());
        ScopedString logs = String::New(m_LogsDirectory);

        s_ManagedFunctions.SetScriptBuildSettingsFptr(sln, proj, logs);

        ScopedString config = String::New(configuration);
        return s_ManagedFunctions.BuildProjectBlockingFptr(config, {}, rebuild);
    }

}