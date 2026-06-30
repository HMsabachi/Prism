#pragma once

#include "Prism/Asset/Asset.h"
#include "Prism/Utilities/FileSystemWatcher.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

namespace Prism
{

    class AssetTypes
    {
    public:
        static void Init();
        static size_t GetAssetTypeID(const std::string& extension);
        static AssetType GetAssetTypeFromExtension(const std::string& extension);

    private:
        static std::map<std::string, AssetType> s_Types;
    };

    struct DirectoryInfo
    {
        std::string DirectoryName;
        std::string FilePath;
        int DirectoryIndex;
        int ParentIndex;
        std::vector<int> ChildrenIndices;
    };

    struct SearchResults
    {
        std::vector<DirectoryInfo> Directories;
        std::vector<Ref<Asset>> Assets;
    };

    class PRISM_API AssetManager
    {
    public:
        using AssetsChangeEventFn = std::function<void()>;
    public:
        static void Init();
        static void SetAssetChangeCallback(const AssetsChangeEventFn& callback);
        static void Shutdown();

        static DirectoryInfo& GetDirectoryInfo(int index);
        static std::vector<Ref<Asset>> GetAssetsInDirectory(int dirIndex);
        static std::vector<std::string> GetDirectoryNames(const std::string& filepath);

        static SearchResults SearchFiles(const std::string& query, const std::string& searchPath);
        static std::string GetParentPath(const std::string& path);

        static bool IsDirectory(const std::string& filepath);

        static AssetHandle GetAssetIDForFile(const std::string& filepath);
        static bool IsAssetHandleValid(AssetHandle assetHandle);

        template<typename T>
        static Ref<T> GetAsset(AssetHandle assetHandle)
        {
            PR_CORE_ASSERT(s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end());
            return (Ref<T>)s_LoadedAssets[assetHandle];
        }

        static bool IsAssetType(AssetHandle assetHandle, AssetType type)
        {
            return s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end() && s_LoadedAssets[assetHandle]->Type == type;
        }
#ifdef MoveFile
#undef MoveFile
#endif
        static bool MoveFile(const std::string& originalPath, const std::string& dest);

        static std::string StripExtras(const std::string& filename);
    private:
        static std::string ParseFilename(const std::string& filepath, const std::string& delim);
        static std::string ParseFileType(std::string_view filename);
        static std::string RemoveExtension(const std::string& filename);

#ifdef RemoveDirectory
#undef RemoveDirectory
#endif
        static void RemoveDirectory(DirectoryInfo& dir);

        static void ImportAsset(const std::string& filepath, bool reimport = false, int parentIndex = -1);
#ifdef CreateMetaFile
#undef CreateMetaFile
#endif
        static void CreateMetaFile(const Ref<Asset>& asset);
        static void LoadMetaData(Ref<Asset>& asset, const std::string& filepath);
        static void ConvertAsset(const std::string& assetPath, const std::string& conversionType);
        static int ProcessDirectory(const std::string& directoryPath, int parentIndex = -1);
        static void ReloadAssets();

        static void OnFileSystemChanged(FileSystemChangedEvent e);

        static int FindParentIndexInChildren(DirectoryInfo& dir, const std::string& dirName);
        static int FindParentIndex(const std::string& filepath);

    private:
        static std::unordered_map<AssetHandle, Ref<Asset>> s_LoadedAssets;
        static std::vector<DirectoryInfo> s_Directories;
        static AssetsChangeEventFn s_AssetsChangeCallback;
    };

}
