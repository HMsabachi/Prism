#include "prpch.h"
#include "FileSystem.h"
#include "FileSystemWatcher.h"

#include <filesystem>

#ifdef PR_PLATFORM_WINDOWS
#include <Windows.h>
#endif

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

    std::string FileSystem::Rename(const std::string& filepath, const std::string& newName)
    {
        FileSystemWatcher::IgnoreNextChange();
        std::filesystem::path p = filepath;
        std::string newFilePath = p.parent_path().string() + "/" + newName + p.extension().string();
#ifdef PR_PLATFORM_WINDOWS
        MoveFileA(filepath.c_str(), newFilePath.c_str());
#else
        std::filesystem::rename(filepath, newFilePath);
#endif
        return newFilePath;
    }

}
