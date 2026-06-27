#pragma once

#include <map>
#include <vector>
#include <string>
#include <string_view>

namespace Prism
{

	enum class AssetType
	{
		Scene, Mesh, Image, Audio, Script, Other
	};

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
		std::string Filename;
		std::string FileType;
		std::string AbsolutePath;
		bool IsFile;

		DirectoryInfo(const std::string& filename, const std::string& fileType, const std::string& absolutePath, bool isFile)
			: Filename(filename), FileType(fileType), AbsolutePath(absolutePath), IsFile(isFile)
		{
		}

		DirectoryInfo(const DirectoryInfo& other)
			: Filename(other.Filename), FileType(other.FileType), AbsolutePath(other.AbsolutePath), IsFile(other.IsFile)
		{
		}

		DirectoryInfo(DirectoryInfo&& other) noexcept
			: Filename(std::move(other.Filename)), FileType(std::move(other.FileType)), AbsolutePath(std::move(other.AbsolutePath)), IsFile(std::move(other.IsFile))
		{
		}

		DirectoryInfo& operator=(const DirectoryInfo& other)
		{
			Filename = other.Filename;
			FileType = other.FileType;
			AbsolutePath = other.AbsolutePath;
			IsFile = other.IsFile;
			return *this;
		}
	};

	class AssetManager
	{
	public:
		AssetManager();

		std::string ParseFilename(std::string_view filepath, int delim);
		std::string ParseFileType(std::string_view filename);

		void HandleAsset(const std::string& filepath);
		void ProcessAsset(const std::string& assetType);
		void ConvertAsset(const std::string& assetPath, const std::string& conversionType);

		std::vector<DirectoryInfo> GetFileSystemContents();
		std::vector<DirectoryInfo> GetDirectoryContents(const std::string& filepath, bool recursive = false);
		std::vector<DirectoryInfo> SearchFiles(const std::string& query, const std::string& searchPath);

		std::string GetParentPath(const std::string& path);

		std::vector<std::string> GetDirectories(const std::string& filepath);

		bool MoveFile(const std::string& originalPath, const std::string& dest);

		std::string StripExtras(const std::string& filename);

	private:
		void ImportAsset(const std::string assetPath, const std::string& assetName);
	};

}
