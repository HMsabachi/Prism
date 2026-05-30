#include "prpch.h"
#include "FileSystem.h"

#include <filesystem>

namespace Prism {

    std::string FileSystem::GetRelativePath(const std::string& absolutePath)
    {
        std::filesystem::path p(absolutePath);
        if (p.is_relative())
            return absolutePath; // already relative

        return std::filesystem::relative(p, std::filesystem::current_path()).string();
    }

    std::string FileSystem::GetAbsolutePath(const std::string& path)
    {
        return std::filesystem::absolute(path).string();
    }

}
