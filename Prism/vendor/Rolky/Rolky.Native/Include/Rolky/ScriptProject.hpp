#pragma once

#include <string>
#include <vector>

namespace Rolky {

    struct ScriptProject
    {
        std::string Name;
        std::string Directory;
        std::string OutputDirectory;
        std::vector<std::string> References;
        std::vector<std::string> Defines;
        std::vector<ScriptProject*> Dependencies;
        bool AllowUnsafe = true;

        std::string GetProjectPath() const;
    };

}
