#include "prpch.h"
#include "AssetManager.h"

#include "EditorAssetManager.h"

namespace Prism
{

    Ref<AssetManagerBase> AssetManager::s_AssetManager;

    void AssetManager::SetAssetManager(Ref<AssetManagerBase> manager)
    {
        s_AssetManager = manager;
    }

    Ref<AssetManagerBase> AssetManager::GetAssetManager()
    {
        PR_CORE_ASSERT(s_AssetManager, "AssetManager not initialized!");
        return s_AssetManager;
    }

    AssetHandle AssetManager::ImportAsset(const std::string& filepath)
    {
        auto* editor = dynamic_cast<EditorAssetManager*>(s_AssetManager.Raw());
        if (editor)
            return editor->ImportAsset(filepath);
        PR_CORE_ERROR("ImportAsset is only available in editor mode");
        return 0;
    }

}
