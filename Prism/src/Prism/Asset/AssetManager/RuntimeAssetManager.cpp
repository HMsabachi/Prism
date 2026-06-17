#include "prpch.h"
#include "RuntimeAssetManager.h"

namespace Prism
{

    RuntimeAssetManager::RuntimeAssetManager()  = default;
    RuntimeAssetManager::~RuntimeAssetManager() = default;

    void RuntimeAssetManager::Shutdown() {}

    AssetType RuntimeAssetManager::GetAssetType(AssetHandle) { return AssetType::None; }
    Ref<Asset> RuntimeAssetManager::GetAsset(AssetHandle) { return nullptr; }
    void RuntimeAssetManager::GetAssetAsync(AssetHandle) {}

    void RuntimeAssetManager::AddMemoryOnlyAsset(Ref<Asset>) {}
    bool RuntimeAssetManager::ReloadData(AssetHandle) { return false; }
    bool RuntimeAssetManager::IsAssetHandleValid(AssetHandle) { return false; }
    bool RuntimeAssetManager::IsAssetLoaded(AssetHandle) { return false; }
    Ref<Asset> RuntimeAssetManager::GetMemoryAsset(AssetHandle) { return nullptr; }
    bool RuntimeAssetManager::IsMemoryAsset(AssetHandle) { return false; }

    void RuntimeAssetManager::RegisterDependency(AssetHandle, AssetHandle) {}
    void RuntimeAssetManager::SyncWithAssetThread() {}

    std::unordered_set<AssetHandle> RuntimeAssetManager::GetAllAssetsWithType(AssetType) { return {}; }
    const std::unordered_map<AssetHandle, Ref<Asset>>& RuntimeAssetManager::GetLoadedAssets()
    {
        static std::unordered_map<AssetHandle, Ref<Asset>> empty;
        return empty;
    }

}
