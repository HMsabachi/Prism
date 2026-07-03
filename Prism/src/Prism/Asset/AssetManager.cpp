#include "prpch.h"
#include "AssetManager.h"

#include "AssetSerializer.h"
#include "Prism/Core/Hash.h"
#include "Prism/Asset/ModelImporter.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Shader/PrismShader.h"
#include "Prism/Utilities/StringUtils.h"
#include "Prism/Utilities/FileSystem.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace Prism {

    void AssetTypes::Init()
    {
        s_Types["psc"] = AssetType::Scene;
        s_Types["fbx"] = AssetType::Mesh;
        s_Types["obj"] = AssetType::Mesh;
        s_Types["blend"] = AssetType::Mesh;
        s_Types["png"] = AssetType::Texture;
        s_Types["hdr"] = AssetType::EnvMap;
        s_Types["ppm"] = AssetType::PhysicsMat;
        s_Types["wav"] = AssetType::Audio;
        s_Types["ogg"] = AssetType::Audio;
        s_Types["cs"] = AssetType::Script;
        s_Types["py"] = AssetType::Script;
        s_Types["Shader"] = AssetType::Shader;
        //s_Types["glsl"] = AssetType::Shader;
    }

    size_t AssetTypes::GetAssetTypeID(const std::string& extension)
    {
        for (auto& kv : s_Types)
        {
            if (kv.first == extension)
                return Hash::GenerateFNVHash64(extension);
        }

        return -1;
    }

    AssetType AssetTypes::GetAssetTypeFromExtension(const std::string& extension)
    {
        return s_Types.find(extension) != s_Types.end() ? s_Types[extension] : AssetType::Other;
    }

    std::map<std::string, AssetType> AssetTypes::s_Types;

    void AssetManager::Init()
    {
        s_ShaderLibrary = Ref<ShaderLibrary>::Create();
        FileSystemWatcher::SetChangeCallback(AssetManager::OnFileSystemChanged);
        ReloadAssets();
    }

    void AssetManager::SetAssetChangeCallback(const AssetsChangeEventFn& callback)
    {
        s_AssetsChangeCallback = callback;
    }

    Ref<ShaderLibrary> AssetManager::GetShaderLibrary()
    {
        return s_ShaderLibrary;
    }

    void AssetManager::Shutdown()
    {
        s_LoadedAssets.clear();
        s_Directories.clear();
        s_ShaderLibrary.Reset();
    }

    DirectoryInfo& AssetManager::GetDirectoryInfo(int index)
    {
        PR_CORE_ASSERT(index >= 0 && index < s_Directories.size());
        return s_Directories[index];
    }

    std::vector<Ref<Asset>> AssetManager::GetAssetsInDirectory(int dirIndex)
    {
        std::vector<Ref<Asset>> results;

        for (auto& asset : s_LoadedAssets)
        {
            if (asset.second && asset.second->ParentDirectory == dirIndex)
                results.push_back(asset.second);
        }

        return results;
    }

    void AssetManager::ConvertAsset(const std::string& assetPath, const std::string& conversionType)
    {
        std::string path = std::filesystem::temp_directory_path().string();
        std::ofstream fileStream(path + "export.py");

        fileStream << "import bpy\n";
        fileStream << "import sys\n";

        if (conversionType == "fbx")
            fileStream << "bpy.ops.export_scene.fbx(filepath=r'" + path + "asset.fbx" + "', axis_forward='-Z', axis_up='Y')\n";

        if (conversionType == "obj")
            fileStream << "bpy.ops.export_scene.obj(filepath=r'" + path + "asset.obj" + "', axis_forward='-Z', axis_up='Y')\n";

        fileStream.close();

        std::string blender_base_path = "C:\\Program Files\\Blender Foundation\\Blender 2.90\\blender.exe";
        std::string p_asset_path = '"' + assetPath + '"';
        std::string p_blender_path = '"' + blender_base_path + '"';
        std::string p_script_path = '"' + path + "export.py" + '"';

        std::string convCommand = '"' + p_blender_path + " " + p_asset_path + " --background --python " + p_script_path + "" + '"';

        PR_CORE_INFO(convCommand.c_str());

        system(convCommand.c_str());
    }

    int AssetManager::FindParentIndexInChildren(DirectoryInfo& dir, const std::string& dirName)
    {
        if (dir.DirectoryName == dirName)
            return dir.DirectoryIndex;

        for (int childIndex : dir.ChildrenIndices)
        {
            DirectoryInfo& child = AssetManager::GetDirectoryInfo(childIndex);

            int dirIndex = FindParentIndexInChildren(child, dirName);

            if (dirIndex != 0)
                return dirIndex;
        }

        return 0;
    }

    int AssetManager::FindParentIndex(const std::string& filepath)
    {
        std::vector<std::string> parts = Utils::SplitString(filepath, "/\\");
        std::string parentFolder = parts[parts.size() - 2];
        DirectoryInfo& assetsDirectory = GetDirectoryInfo(0);
        return FindParentIndexInChildren(assetsDirectory, parentFolder);
    }

    void AssetManager::OnFileSystemChanged(FileSystemChangedEvent e)
    {
        e.NewName = Utils::RemoveExtension(e.NewName);
        e.OldName = Utils::RemoveExtension(e.OldName);

        int parentIndex = FindParentIndex(e.FilePath);

        if (e.Action == FileSystemAction::Added)
        {
            if (e.IsDirectory)
                ProcessDirectory(e.FilePath, parentIndex);
            else
                ImportAsset(e.FilePath, false, parentIndex);
        }

        if (e.Action == FileSystemAction::Modified)
        {
            if (!e.IsDirectory)
                ImportAsset(e.FilePath, true, parentIndex);
        }

        if (e.Action == FileSystemAction::Rename)
        {
            if (e.IsDirectory)
            {
                for (auto& dir : s_Directories)
                {
                    if (dir.DirectoryName == e.OldName)
                    {
                        dir.FilePath = e.FilePath;
                        dir.DirectoryName = e.NewName;
                    }
                }
            }
            else
            {
                for (auto it = s_LoadedAssets.begin(); it != s_LoadedAssets.end(); it++)
                {
                    if (it->second->FileName == e.OldName)
                    {
                        it->second->FilePath = e.FilePath;
                        it->second->FileName = e.NewName;
                    }
                }
            }
        }

        if (e.Action == FileSystemAction::Delete)
        {
            if (e.IsDirectory)
            {
            }
            else
            {
                for (auto it = s_LoadedAssets.begin(); it != s_LoadedAssets.end(); it++)
                {
                    if (it->second->FilePath != e.FilePath)
                        continue;

                    s_LoadedAssets.erase(it);
                    break;
                }
            }
        }

        s_AssetsChangeCallback();
    }

    SearchResults AssetManager::SearchFiles(const std::string& query, const std::string& searchPath)
    {
        SearchResults results;

        if (!searchPath.empty())
        {
            for (const auto& dir : s_Directories)
            {
                if (dir.DirectoryName.find(query) != std::string::npos && dir.FilePath.find(searchPath) != std::string::npos)
                {
                    results.Directories.push_back(dir);
                }
            }

            for (const auto&[key, asset] : s_LoadedAssets)
            {
                if (asset->FileName.find(query) != std::string::npos && asset->FilePath.find(searchPath) != std::string::npos)
                {
                    results.Assets.push_back(asset);
                }
            }
        }

        return results;
    }

    std::string AssetManager::GetParentPath(const std::string& path)
    {
        return std::filesystem::path(path).parent_path().string();
    }

    bool AssetManager::IsDirectory(const std::string& filepath)
    {
        for (auto& dir : s_Directories)
        {
            if (dir.FilePath == filepath)
                return true;
        }

        return false;
    }

    AssetHandle AssetManager::GetAssetIDForFile(const std::string& filepath)
    {
        std::string normalizedPath = std::filesystem::path(filepath).string();
        for (auto&[id, asset] : s_LoadedAssets)
        {
            if (asset->FilePath == normalizedPath)
                return id;
        }

        return 0;
    }

    bool AssetManager::IsAssetHandleValid(AssetHandle assetHandle)
    {
        return assetHandle != 0 && s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end();
    }

    void AssetManager::RemoveDirectory(int directory)
    {
        for (auto it = s_Directories.begin(); it != s_Directories.end(); )
        {
            if (it->DirectoryIndex == directory)
            {
                for (auto child : it->ChildrenIndices)
                    RemoveDirectory(child);

                auto assets = GetAssetsInDirectory(directory);
                for (auto& asset : assets)
                {
                    RemoveAsset(asset->Handle);
                }

                it = s_Directories.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    void AssetManager::RemoveAsset(AssetHandle assetHandle)
    {
        s_LoadedAssets.erase(assetHandle);
    }

    void AssetManager::Rename(Ref<Asset>& asset, const std::string& newName)
    {
        std::string newFilePath = FileSystem::Rename(asset->FilePath, newName);
        std::string oldFilePath = asset->FilePath;
        asset->FilePath = newFilePath;
        asset->FileName = newName;

        if (std::filesystem::exists(oldFilePath + ".meta"))
        {
            FileSystem::Rename(oldFilePath + ".meta", newName + "." + asset->Extension);
            AssetSerializer::CreateMetaFile(asset);
        }
    }

    void AssetManager::Rename(int directoryIndex, const std::string& newName)
    {
        DirectoryInfo& dir = GetDirectoryInfo(directoryIndex);
        std::string newFilePath = FileSystem::Rename(dir.FilePath, newName);
        dir.FilePath = newFilePath;
        dir.DirectoryName = newName;
    }

    std::string AssetManager::StripExtras(const std::string& filename)
    {
        std::vector<std::string> out;
        size_t start;
        size_t end = 0;

        while ((start = filename.find_first_not_of(".", end)) != std::string::npos)
        {
            end = filename.find(".", start);
            out.push_back(filename.substr(start, end - start));
        }

        if (out[0].length() >= 10)
        {
            auto cutFilename = out[0].substr(0, 9) + "...";
            return cutFilename;
        }

        auto filenameLength = out[0].length();
        auto paddingToAdd = 9 - filenameLength;

        std::string newFileName;

        for (int i = 0; i <= paddingToAdd; i++)
        {
            newFileName += " ";
        }

        newFileName += out[0];

        return newFileName;
    }

    void AssetManager::ImportAsset(const std::string& filepath, bool reimport, int parentIndex)
    {
        std::string path = filepath;
        std::string extension = Utils::GetExtension(filepath);
        if (extension == "meta")
            return;

        AssetType type = AssetTypes::GetAssetTypeFromExtension(extension);
        Ref<Asset> asset = AssetSerializer::LoadAssetInfo(path, parentIndex, type);

        if (s_LoadedAssets.find(asset->Handle) != s_LoadedAssets.end())
        {
            if (s_LoadedAssets[asset->Handle]->IsDataLoaded)
            {
                asset = AssetSerializer::LoadAssetData(asset);
            }
        }

        s_LoadedAssets[asset->Handle] = asset;
    }

    int AssetManager::ProcessDirectory(const std::string& directoryPath, int parentIndex)
    {
        DirectoryInfo dirInfo;
        dirInfo.DirectoryName = std::filesystem::path(directoryPath).filename().string();
        dirInfo.ParentIndex = parentIndex;
        dirInfo.FilePath = directoryPath;
        s_Directories.push_back(dirInfo);
        int currentIndex = (int)s_Directories.size() - 1;
        s_Directories[currentIndex].DirectoryIndex = currentIndex;

        if (parentIndex != -1)
            s_Directories[parentIndex].ChildrenIndices.push_back(currentIndex);

        for (auto entry : std::filesystem::directory_iterator(directoryPath))
        {
            if (entry.is_directory())
                ProcessDirectory(entry.path().string(), currentIndex);
            else
                ImportAsset(entry.path().string(), false, currentIndex);
        }

        return currentIndex;
    }

    void AssetManager::ReloadAssets()
    {
        ProcessDirectory("Assets");
        s_ShaderLibrary->LoadAll("Assets");
    }

    std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::s_LoadedAssets;
    std::vector<DirectoryInfo> AssetManager::s_Directories;
    AssetManager::AssetsChangeEventFn AssetManager::s_AssetsChangeCallback;
    Ref<ShaderLibrary> AssetManager::s_ShaderLibrary;

}
