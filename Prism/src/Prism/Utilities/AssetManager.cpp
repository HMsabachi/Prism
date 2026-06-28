#include "prpch.h"
#include "AssetManager.h"

#include <filesystem>

namespace Prism {

	void AssetTypes::Init()
	{
		s_Types["psc"] = AssetType::Scene;
		s_Types["fbx"] = AssetType::Mesh;
		s_Types["obj"] = AssetType::Mesh;
		s_Types["png"] = AssetType::Image;
		s_Types["blend"] = AssetType::Mesh;
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
				return std::hash<std::string>()(extension);
		}

		return -1;
	}

	AssetType AssetTypes::GetAssetTypeFromExtension(const std::string& extension)
	{
		return s_Types.find(extension) != s_Types.end() ? s_Types[extension] : AssetType::Other;
	}

	std::map<std::string, AssetType> AssetTypes::s_Types;

	AssetManager::AssetManager()
	{

	}

	std::string AssetManager::ParseFilename(std::string_view filepath, int delim)
	{
		std::vector<std::string> out;
		size_t start;
		size_t end = 0;

		while ((start = filepath.find_first_not_of((char)delim, end)) != std::string_view::npos)
		{
			end = filepath.find((char)delim, start);
			out.emplace_back(filepath.substr(start, end - start));
		}

		return out[out.size() - 1];
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

	void AssetManager::HandleAsset(const std::string& filepath)
	{

	}

	void AssetManager::ProcessAsset(const std::string& assetType)
	{
		std::string filename = ParseFilename(assetType, '/\\');
		std::string filetype = ParseFileType(assetType);

		if (filetype == "blend")
		{
			ConvertAsset(assetType, "fbx");
		}
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

	std::vector<DirectoryInfo> AssetManager::GetFileSystemContents()
	{
		std::string path = "assets";
		std::vector<DirectoryInfo> directories;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			bool isDir = std::filesystem::is_directory(entry);

			std::string dir_data = ParseFilename(entry.path().string(), '/\\');
			std::string fileExt = ParseFileType(dir_data);
			directories.emplace_back(dir_data, fileExt, entry.path().string(), !isDir);
		}

		return directories;
	}

	std::vector<DirectoryInfo> AssetManager::GetDirectoryContents(const std::string& filepath, bool recursive)
	{
		std::vector<DirectoryInfo> directories;

		if (recursive)
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(filepath))
			{
				bool isDir = std::filesystem::is_directory(entry);
				std::string dir_data = ParseFilename(entry.path().string(), '/\\');
				directories.emplace_back(dir_data, ".prism", entry.path().string(), !isDir);
			}
		}
		else
		{
			for (const auto& entry : std::filesystem::directory_iterator(filepath))
			{
				bool isDir = std::filesystem::is_directory(entry);
				std::string dir_data = ParseFilename(entry.path().string(), '/\\');
				std::string fileExt = ParseFileType(dir_data);
				directories.emplace_back(dir_data, fileExt, entry.path().string(), !isDir);
			}
		}

		return directories;
	}

	std::vector<DirectoryInfo> AssetManager::SearchFiles(const std::string& query, const std::string& searchPath)
	{
		std::vector<DirectoryInfo> result;

		if (!searchPath.empty())
		{
			std::vector<DirectoryInfo> contents = GetDirectoryContents(searchPath, true);

			for (auto& entry : contents)
			{
				if (entry.Filename.find(query) != std::string::npos)
					result.emplace_back(std::move(entry));
			}
		}

		return result;
	}

	std::string AssetManager::GetParentPath(const std::string& path)
	{
		return std::filesystem::path(path).parent_path().string();
	}

	std::vector<std::string> AssetManager::GetDirectories(const std::string& filepath)
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
		std::string newPath = dest + "/" + ParseFilename(originalPath, '/\\');
		return std::filesystem::exists(newPath);
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

	void AssetManager::ImportAsset(const std::string assetPath, const std::string& assetName)
	{

	}
}
