#pragma once

#include "Asset.h"
#include "AssetSerializer.h"
#include "Prism/Utilities/FileSystemWatcher.h"
#include "Prism/Utilities/StringUtils.h"
#include "Prism/Core/Hash.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

namespace Prism
{

    class ShaderLibrary;

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

        static Ref<ShaderLibrary> GetShaderLibrary();

        static DirectoryInfo& GetDirectoryInfo(int index);
        static std::vector<Ref<Asset>> GetAssetsInDirectory(int dirIndex);

        static SearchResults SearchFiles(const std::string& query, const std::string& searchPath);
        static std::string GetParentPath(const std::string& path);

        static bool IsDirectory(const std::string& filepath);

        static AssetHandle GetAssetIDForFile(const std::string& filepath);
        static bool IsAssetHandleValid(AssetHandle assetHandle);

        static void Rename(Ref<Asset>& asset, const std::string& newName);
        static void Rename(int directoryIndex, const std::string& newName);

        template<typename T, typename... Args>
        static Ref<T> CreateAsset(const std::string& filename, AssetType type, int directoryIndex, Args&&... args)
        {
            static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");

            auto& directory = GetDirectoryInfo(directoryIndex);

            Ref<T> asset = Ref<T>::Create(std::forward<Args>(args)...);
            asset->Type = type;
            asset->FilePath = directory.FilePath + "/" + filename;
            asset->FileName = Utils::RemoveExtension(Utils::GetFilename(asset->FilePath));
            asset->Extension = Utils::GetExtension(filename);
            asset->ParentDirectory = directoryIndex;
            asset->Handle = UUID();
            s_LoadedAssets[asset->Handle] = asset;

            AssetSerializer::SerializeAsset(asset);

            return asset;
        }

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

        static std::string StripExtras(const std::string& filename);
    private:
        static void ImportAsset(const std::string& filepath, bool reimport = false, int parentIndex = -1);
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
        static Ref<ShaderLibrary> s_ShaderLibrary;
    };

}
