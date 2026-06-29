#pragma once

#include <functional>
#include <string>

namespace Prism
{

    enum class FileSystemAction
    {
        Added, Rename, Modified, Delete
    };

    struct FileSystemChangedEvent
    {
        FileSystemAction Action;
        std::string Filepath;
        std::string OldName;
        std::string NewName;
        bool IsDirectory;
    };

    class PRISM_API FileSystemWatcher
    {
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
