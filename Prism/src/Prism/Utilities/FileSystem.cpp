#include "prpch.h"
#include "FileSystem.h"
#include "FileSystemWatcher.h"

#include <filesystem>

#ifdef PR_PLATFORM_WINDOWS
#include <Windows.h>
#include <shellapi.h>
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

    bool FileSystem::DeleteFile(const std::string& filepath)
    {
        std::string fp = filepath;
        fp.append(1, '\0');
        SHFILEOPSTRUCTA file_op;
        file_op.hwnd = NULL;
        file_op.wFunc = FO_DELETE;
        file_op.pFrom = fp.c_str();
        file_op.pTo = "";
        file_op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        file_op.fAnyOperationsAborted = false;
        file_op.hNameMappings = 0;
        file_op.lpszProgressTitle = "";
        return SHFileOperationA(&file_op) == 0;
    }

}
