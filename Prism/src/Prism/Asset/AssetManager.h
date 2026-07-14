#pragma once

#include "Asset.h"
#include "AssetImporter.h"
#include "Prism/Utilities/FileSystem.h"
#include "Prism/Utilities/StringUtils.h"
#include "Prism/Core/Hash.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

namespace std::filesystem
{
    class path;
}

namespace Prism
{

    class ShaderLibrary;

    class PRISM_API AssetManager
    {
    public:
        using AssetsChangeEventFn = std::function<void()>;

        struct AssetMetadata
        {
            AssetHandle Handle;
            std::string FilePath;
            AssetType Type;
        };
    public:
        static void Init();
        static void SetAssetChangeCallback(const AssetsChangeEventFn& callback);
        static void Shutdown();

        static Ref<ShaderLibrary> GetShaderLibrary();

        static std::vector<Ref<Asset>> GetAssetsInDirectory(AssetHandle directoryHandle);

        static std::vector<Ref<Asset>> SearchAssets(const std::string& query, const std::string& searchPath, AssetType desiredType = AssetType::None);
        static std::string GetParentPath(const std::string& path);

        static bool IsDirectory(const std::string& filepath);

        static AssetHandle GetAssetHandleFromFilePath(const std::string& filepath);
        static bool IsAssetHandleValid(AssetHandle assetHandle);

        static void Rename(AssetHandle assetHandle, const std::string& newName);

        static void RemoveAsset(AssetHandle assetHandle);

        static AssetType GetAssetTypeForFileType(const std::string& extension);

        template<typename T, typename... Args>
        static Ref<T> CreateNewAsset(const std::string& filename, AssetType type, AssetHandle directoryHandle, Args&&... args)
        {
            static_assert(std::is_base_of<Asset, T>::value, "CreateNewAsset only works for types derived from Asset");

            auto directory = GetAsset<Directory>(directoryHandle);

            Ref<T> asset = Ref<T>::Create(std::forward<Args>(args)...);
            asset->Type = type;
            asset->FilePath = directory->FilePath + "/" + filename;
            asset->FileName = Utils::RemoveExtension(Utils::GetFilename(asset->FilePath));
            asset->Extension = Utils::GetExtension(filename);
            asset->ParentDirectory = directoryHandle;
            asset->Handle = AssetHandle();
            asset->IsDataLoaded = true;
            s_LoadedAssets[asset->Handle] = asset;

            AssetImporter::Serialize(asset);

            AssetMetadata metadata;
            metadata.Handle = asset->Handle;
            metadata.FilePath = asset->FilePath;
            metadata.Type = asset->Type;
            s_AssetRegistry[asset->FilePath] = metadata;
            UpdateRegistryCache();

            return asset;
        }

        template<typename T>
        static Ref<T> GetAsset(AssetHandle assetHandle, bool loadData = true)
        {
            PR_CORE_ASSERT(s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end());
            Ref<Asset>& asset = s_LoadedAssets[assetHandle];

            if (!asset->IsDataLoaded && loadData)
                AssetImporter::TryLoadData(asset);

            return asset.As<T>();
        }

        template<typename T>
        static Ref<T> GetAsset(const std::string& filepath, bool loadData = true)
        {
            return GetAsset<T>(GetAssetHandleFromFilePath(filepath), loadData);
        }

        static bool IsAssetType(AssetHandle assetHandle, AssetType type)
        {
            return s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end() && s_LoadedAssets[assetHandle]->Type == type;
        }

        static std::string StripExtras(const std::string& filename);
    private:
        static void LoadAssetRegistry();
        static Ref<Asset> CreateAsset(const std::string& filepath, AssetType type, AssetHandle parentHandle);
        static void ImportAsset(const std::string& filepath, AssetHandle parentHandle);
        static AssetHandle ProcessDirectory(const std::string& directoryPath, AssetHandle parentHandle);
        static void ReloadAssets();
        static void UpdateRegistryCache();

        static void OnFileSystemChanged(FileSystemChangedEvent e);

        static AssetHandle FindParentHandleInChildren(Ref<Directory>& dir, const std::string& dirName);
        static AssetHandle FindParentHandle(const std::string& filepath);

    private:
        static std::unordered_map<AssetHandle, Ref<Asset>> s_LoadedAssets;
        static std::unordered_map<std::string, AssetMetadata> s_AssetRegistry;
        static AssetsChangeEventFn s_AssetsChangeCallback;
        static Ref<ShaderLibrary> s_ShaderLibrary;
    };

}
