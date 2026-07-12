#pragma once

#include "ScriptSolution.hpp"

#include <string>

namespace Rolky {

    class BuildManager
    {
    public:
        void SetLogsDirectory(std::string_view logsDirectory);

        bool Build(ScriptSolution& solution, std::string_view configuration = "Debug", bool rebuild = false);

    private:
        std::string m_LogsDirectory;
    };

}
