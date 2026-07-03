#pragma once

#include <functional>
#include <string>
#include "Prism/Core/Core.h"

namespace Prism
{

    enum class FileSystemAction
    {
        Added, Rename, Modified, Delete
    };

    struct FileSystemChangedEvent
    {
        FileSystemAction Action;
        std::string FilePath;
        std::string OldName;
        std::string NewName;
        bool IsDirectory;
    };

    class PRISM_API FileSystem
    {
    public:
        static bool CreateFolder(const std::string& filepath);
        static bool Exists(const std::string& filepath);
        static std::string Rename(const std::string& filepath, const std::string& newName);
        static bool DeleteFile(const std::string& filepath);

        /** Convert absolute path to relative (relative to working directory) */
        static std::string GetRelativePath(const std::string& absolutePath);

        /** Convert any path to absolute (resolves relative paths against working directory) */
        static std::string GetAbsolutePath(const std::string& path);

    public:
        using FileSystemChangedCallbackFn = std::function<void(FileSystemChangedEvent)>;

        static void SetChangeCallback(const FileSystemChangedCallbackFn& callback);
        static void StartWatching();
        static void StopWatching();

    private:
        static unsigned long Watch(void* param);

    private:
        static FileSystemChangedCallbackFn s_Callback;
    };

}
