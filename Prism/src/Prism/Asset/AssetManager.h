#pragma once

#include "Asset.h"
#include "AssetSerializer.h"
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

    class AssetTypes
    {
    public:
        static void Init();
        static size_t GetAssetTypeID(const std::string& extension);
        static AssetType GetAssetTypeFromExtension(const std::string& extension);

    private:
        static std::map<std::string, AssetType> s_Types;
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

        static std::vector<Ref<Asset>> GetAssetsInDirectory(AssetHandle directoryHandle);

        static std::vector<Ref<Asset>> SearchFiles(const std::string& query, const std::string& searchPath);
        static std::string GetParentPath(const std::string& path);

        static bool IsDirectory(const std::string& filepath);

        static AssetHandle GetAssetIDForFile(const std::string& filepath);
        static bool IsAssetHandleValid(AssetHandle assetHandle);

        static void Rename(Ref<Asset>& asset, const std::string& newName);

        static void RemoveAsset(AssetHandle assetHandle);

        template<typename T, typename... Args>
        static Ref<T> CreateAsset(const std::string& filename, AssetType type, AssetHandle directoryHandle, Args&&... args)
        {
            static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");

            auto directory = GetAsset<Directory>(directoryHandle);

            Ref<T> asset = Ref<T>::Create(std::forward<Args>(args)...);
            asset->Type = type;
            asset->FilePath = directory->FilePath + "/" + filename;
            asset->FileName = Utils::RemoveExtension(Utils::GetFilename(asset->FilePath));
            asset->Extension = Utils::GetExtension(filename);
            asset->ParentDirectory = directoryHandle;
            asset->Handle = UUID();
            asset->IsDataLoaded = true;
            s_LoadedAssets[asset->Handle] = asset;

            AssetSerializer::SerializeAsset(asset);

            return asset;
        }

        template<typename T>
        static Ref<T> GetAsset(AssetHandle assetHandle, bool loadData = true)
        {
            PR_CORE_ASSERT(s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end());
            Ref<Asset> asset = s_LoadedAssets[assetHandle];

            if (!asset->IsDataLoaded && loadData)
            {
                asset = AssetSerializer::LoadAssetData(asset);
                s_LoadedAssets[assetHandle] = asset;
            }

            return asset.As<T>();
        }

        static bool IsAssetType(AssetHandle assetHandle, AssetType type)
        {
            return s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end() && s_LoadedAssets[assetHandle]->Type == type;
        }

        static std::string StripExtras(const std::string& filename);
    private:
        static void ImportAsset(const std::string& filepath, AssetHandle parentHandle);
        static AssetHandle ProcessDirectory(const std::string& directoryPath, AssetHandle parentHandle);
        static void ReloadAssets();

        static void OnFileSystemChanged(FileSystemChangedEvent e);

        static AssetHandle FindParentHandleInChildren(Ref<Directory>& dir, const std::string& dirName);
        static AssetHandle FindParentHandle(const std::string& filepath);

    private:
        static std::unordered_map<AssetHandle, Ref<Asset>> s_LoadedAssets;
        static AssetsChangeEventFn s_AssetsChangeCallback;
        static Ref<ShaderLibrary> s_ShaderLibrary;
    };

}
