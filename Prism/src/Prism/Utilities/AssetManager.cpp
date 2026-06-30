#include "prpch.h"
#include "AssetManager.h"

#include "Prism/Core/Hash.h"
#include "Prism/Asset/ModelImporter.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Texture.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace Prism {

    static std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters)
    {
        size_t start = 0;
        size_t end = string.find_first_of(delimiters);
        std::vector<std::string> result;

        while (end != std::string::npos)
        {
            result.push_back(string.substr(start, end - start));
            start = end + 1;
            end = string.find_first_of(delimiters, start);
        }

        result.push_back(string.substr(start));
        return result;
    }

    void AssetTypes::Init()
    {
        s_Types["psc"] = AssetType::Scene;
        s_Types["fbx"] = AssetType::Mesh;
        s_Types["obj"] = AssetType::Mesh;
        s_Types["blend"] = AssetType::Mesh;
        s_Types["png"] = AssetType::Texture;
        s_Types["hdr"] = AssetType::EnvMap;
        s_Types["wav"] = AssetType::Audio;
        s_Types["ogg"] = AssetType::Audio;
        s_Types["cs"] = AssetType::Script;
        s_Types["py"] = AssetType::Script;
        s_Types["Shader"] = AssetType::Shader;
        s_Types["glsl"] = AssetType::Shader;
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
        FileSystemWatcher::SetChangeCallback(AssetManager::OnFileSystemChanged);
        ReloadAssets();
    }

    void AssetManager::SetAssetChangeCallback(const AssetsChangeEventFn& callback)
    {
        s_AssetsChangeCallback = callback;
    }

    void AssetManager::Shutdown()
    {
        s_LoadedAssets.clear();
        s_Directories.clear();
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
            if (asset.second->ParentDirectory == dirIndex)
                results.push_back(asset.second);
        }

        return results;
    }

    std::string AssetManager::ParseFilename(const std::string& filepath, const std::string& delim)
    {
        std::vector<std::string> parts = SplitString(filepath, delim);
        return parts[parts.size() - 1];
    }

    std::string AssetManager::ParseFileType(std::string_view filename)
    {
        size_t start;
        size_t end = 0;
        std::vector<std::string> out;

        while ((start = filename.find_first_not_of('.', end)) != std::string_view::npos)
        {
            end = filename.find('.', start);
            out.emplace_back(filename.substr(start, end - start));
        }

        return out[out.size() - 1];
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
        std::vector<std::string> parts = SplitString(filepath, "/\\");
        std::string parentFolder = parts[parts.size() - 2];
        DirectoryInfo& assetsDirectory = GetDirectoryInfo(0);
        return FindParentIndexInChildren(assetsDirectory, parentFolder);
    }

    void AssetManager::OnFileSystemChanged(FileSystemChangedEvent e)
    {
        e.NewName = RemoveExtension(e.NewName);
        e.OldName = RemoveExtension(e.OldName);

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
        for (auto&[id, asset] : s_LoadedAssets)
        {
            if (asset->FilePath == filepath)
                return id;
        }

        return 0;
    }

    bool AssetManager::IsAssetHandleValid(AssetHandle assetHandle)
    {
        return s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end();
    }

    std::vector<std::string> AssetManager::GetDirectoryNames(const std::string& filepath)
    {
        std::vector<std::string> result;
        size_t start;
        size_t end = 0;

        while ((start = filepath.find_first_not_of("/\\", end)) != std::string::npos)
        {
            end = filepath.find("/\\", start);
            result.push_back(filepath.substr(start, end - start));
        }

        return result;
    }

    bool AssetManager::MoveFile(const std::string& originalPath, const std::string& dest)
    {
        std::filesystem::rename(originalPath, dest);
        std::string newPath = dest + "/" + ParseFilename(originalPath, "/\\");
        return std::filesystem::exists(newPath);
    }

    std::string AssetManager::RemoveExtension(const std::string& filename)
    {
        std::string newName;
        size_t end = filename.find_last_of('.');

        newName = filename.substr(0, end);
        return newName;
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
        std::string extension = ParseFileType(filepath);
        if (extension == "meta")
            return;

        Ref<Asset> asset;
        AssetType type = AssetTypes::GetAssetTypeFromExtension(extension);

        switch (type)
        {
            case AssetType::Scene:
            {
                asset = Ref<Asset>::Create();
                break;
            }
            case AssetType::Mesh:
            {
                if (extension == "blend")
                    asset = Ref<Asset>::Create();
                else
                    asset = Ref<Asset>::Create();
                break;
            }
            case AssetType::Texture:
            {
                asset = Ref<Asset>::Create();
                break;
            }
            case AssetType::EnvMap:
            {
                asset = Ref<Asset>::Create();
                break;
            }
            case AssetType::Audio:
            {
                break;
            }
            case AssetType::Script:
            {
                asset = Ref<Asset>::Create();
                break;
            }
            case AssetType::Shader:
            case AssetType::Other:
            {
                asset = Ref<Asset>::Create();
                break;
            }
        }

        asset->Handle = Hash::GenerateFNVHash64(filepath);
        asset->FilePath = filepath;
        asset->FileName = RemoveExtension(ParseFilename(filepath, "/\\"));
        asset->Extension = extension;
        asset->ParentDirectory = parentIndex;
        asset->Type = type;

        bool hasMeta = std::filesystem::exists(filepath + ".meta");
        if (hasMeta)
            LoadMetaData(asset, filepath + ".meta");

        std::replace(asset->FilePath.begin(), asset->FilePath.end(), '\\', '/');

        if (!hasMeta || reimport)
            CreateMetaFile(asset);

        s_LoadedAssets[asset->Handle] = asset;
    }

    void AssetManager::CreateMetaFile(const Ref<Asset>& asset)
    {
        if (std::filesystem::exists(asset->FilePath + ".meta"))
            return;

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Asset" << YAML::Value << asset->Handle;
        out << YAML::Key << "FileName" << YAML::Value << asset->FileName;
        out << YAML::Key << "FilePath" << YAML::Value << asset->FilePath;
        out << YAML::Key << "Extension" << YAML::Value << asset->Extension;
        out << YAML::Key << "Directory" << YAML::Value << asset->ParentDirectory;
        out << YAML::Key << "Type" << YAML::Value << (int)asset->Type;
        out << YAML::EndMap;

        std::ofstream fout(asset->FilePath + ".meta");
        fout << out.c_str();
    }

    void AssetManager::LoadMetaData(Ref<Asset>& asset, const std::string& filepath)
    {
        std::ifstream stream(filepath);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Asset"])
        {
            PR_CORE_ERROR("Invalid Meta File Format: {0}", filepath);
            return;
        }

        asset->Handle = data["Asset"].as<AssetHandle>();
        asset->FileName = data["FileName"].as<std::string>();
        asset->FilePath = data["FilePath"].as<std::string>();
        asset->Extension = data["Extension"].as<std::string>();
        asset->ParentDirectory = data["Directory"].as<int>();
        asset->Type = (AssetType)data["Type"].as<int>();
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
        ProcessDirectory("assets");
    }

    std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::s_LoadedAssets;
    std::vector<DirectoryInfo> AssetManager::s_Directories;
    AssetManager::AssetsChangeEventFn AssetManager::s_AssetsChangeCallback;

}
