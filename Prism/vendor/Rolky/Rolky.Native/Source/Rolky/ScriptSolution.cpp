#include "Rolky/ScriptSolution.hpp"
#include "Rolky/ProjectDesc.hpp"
#include "Rolky/Array.hpp"
#include "Rolky/String.hpp"

#include "RolkyManagedFunctions.hpp"

#include <sstream>

namespace Rolky {

    ScriptProject& ScriptSolution::AddProject(std::string_view name)
    {
        auto& project = Projects.emplace_back();
        project.Name = name;
        project.Directory = Directory;
        project.OutputDirectory = OutputDirectory;
        return project;
    }

    ScriptProject& ScriptSolution::GetProject(std::string_view name)
    {
        for (auto& p : Projects)
        {
            if (p.Name == name)
                return p;
        }
        throw std::runtime_error("Project not found");
    }

    void ScriptSolution::Generate()
    {
        auto descs = Array<ProjectDesc>::New(Projects.size());

        for (size_t i = 0; i < Projects.size(); i++)
        {
            auto& p = Projects[i];
            auto& d = descs[i];

            d.Name = String::New(p.Name);
            d.Directory = String::New(p.Directory);
            d.OutputDirectory = String::New(p.OutputDirectory);
            d.AllowUnsafe = p.AllowUnsafe;

            std::ostringstream refs;
            for (size_t j = 0; j < p.References.size(); j++)
            {
                if (j > 0) refs << ";";
                refs << p.References[j];
            }
            d.ReferencePaths = String::New(refs.str());

            std::ostringstream defines;
            for (size_t j = 0; j < p.Defines.size(); j++)
            {
                if (j > 0) defines << ";";
                defines << p.Defines[j];
            }
            d.Defines = String::New(defines.str());

            std::ostringstream sources;
            for (size_t j = 0; j < p.SourceFiles.size(); j++)
            {
                if (j > 0) sources << ";";
                sources << p.SourceFiles[j];
            }
            d.SourceFiles = String::New(sources.str());
        }

        ScopedString dir = String::New(Directory);
        ScopedString name = String::New(Name);
        s_ManagedFunctions.GenerateSolutionFptr(dir, name, descs);
    }

    std::string ScriptSolution::GetSolutionPath() const
    {
        return Directory + "/" + Name + ".sln";
    }

}
