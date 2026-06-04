#pragma once

#include "ScriptProject.hpp"

#include <string>
#include <vector>

namespace Rolky {

    struct ScriptSolution
    {
        std::string Name;
        std::string Directory;
        std::string OutputDirectory;
        std::vector<ScriptProject> Projects;

        ScriptProject& AddProject(std::string_view name);
        ScriptProject* GetProject(std::string_view name);

        void Generate();
        std::string GetSolutionPath() const;
    };

}
