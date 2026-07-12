#include "Rolky/ScriptProject.hpp"

namespace Rolky {

    std::string ScriptProject::GetProjectPath() const
    {
        return Directory + "/" + Name + ".csproj";
    }

}
