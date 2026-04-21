workspace "Prism"
	architecture "x64"
	--startproject "Sandbox"
	startproject "PrismEditor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Prism/vendor/GLFW/include"
IncludeDir["Glad"] = "Prism/vendor/Glad/include"
IncludeDir["ImGui"] = "Prism/vendor/imgui"
IncludeDir["glm"] = "Prism/vendor/glm"
IncludeDir["stb_image"] = "Prism/vendor/stb_image"
IncludeDir["PrismShaderParser"] = "Prism/vendor/PrismShaderParser/src"
IncludeDir["nethost"] = "Prism/vendor/nethost"

LibraryDir = {}
LibraryDir["nethost"] = "Prism/vendor/nethost"
group "Dependencies"
	include "Prism/vendor/GLFW"
	include "Prism/vendor/Glad"
	include "Prism/vendor/imgui"
	include "Prism/vendor/PrismShaderParser"
group ""

group "Core"

project "Prism"
	location "Prism"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

	defines { "PR_DYNAMIC_LINK" , "_CRT_SECURE_NO_WARNINGS" }

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "prpch.h"
	pchsource "Prism/src/prpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.h",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.PrismShaderParser}",
		"%{prj.name}/vendor/assimp/include",
		"%{IncludeDir.nethost}"
	}

	libdirs
	{
		-- ==================== .NET 9 nethost 库路径 ====================
		"C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/9.0.*/build/native"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"PrismShaderParser",
		"opengl32.lib",
		"dwmapi.lib"
	}
	linkoptions { "/WHOLEARCHIVE:ImGui" }

	-- ==================== C# 脚本系统支持 ====================
	-- 为 Prism C++ 项目添加 Scripting 目录，并链接 .NET 9 hostfxr
	libdirs
	{
		"%{IncludeDir.nethost}"           -- 新增：告诉链接器去这里找 nethost.lib
	}
	files {
		"%{prj.name}/src/Scripting/**.h",
		"%{prj.name}/src/Scripting/**.cpp"
	}
	includedirs { "%{prj.name}/src/Scripting" }
	links { "nethost" }   -- .NET 9 嵌入运行时必需的库

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		buildoptions { "/utf-8" }

		defines
		{
			"PR_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/PrismEditor/\""),
			('{COPY} "vendor/nethost/*.dll" "../bin/%{outputdir}/PrismEditor/"')
		}
	filter "configurations:Debug"
		defines "PR_DEBUG"
		optimize "Off"    
		symbols "On"
		runtime "Debug"
		links
		{
			"Prism/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
		}
		

	filter "configurations:Release"
		defines "PR_RELEASE"
		optimize "On"
		runtime "Release"
		links
		{
			"Prism/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}


	filter "configurations:Dist"
		defines "PR_DIST"
		optimize "On"
		runtime "Release"
		links
		{
			"Prism/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}

-- ==================== Prism.Scripting C# 类库（.NET 9） ====================
project "Prism.Scripting"
	location "Prism.Scripting"
	kind "SharedLib"
	language "C#"
	dotnetframework "net9.0"          -- 使用 .NET 9
	namespace "Prism"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.cs",       
	}

	-- 让 C# 可以 P/Invoke 调用 Prism.dll
	links { "Prism" }

	-- 允许 unsafe 代码（后面绑定结构体时会用到）
	clr "Unsafe"

	-- 输出路径和 PrismEditor 保持一致
	postbuildcommands
	{
		'{COPY} "%{cfg.buildtarget.relpath}" "../bin/' .. outputdir .. '/PrismEditor/"'
	}

group ""

-- PrismEditor 项目（保持你原来的配置）
project "PrismEditor"
	location "PrismEditor"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	defines "PR_DYNAMIC_LINK"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	postbuildcommands
	{
    	'{COPYDIR} "%{prj.location}/Assets" "%{cfg.targetdir}/Assets"',
	}
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Prism/vendor/spdlog/include",
		"Prism/src",
		"Prism/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.PrismShaderParser}",
		"%{IncludeDir.nethost}"

	}

	links
	{
		"Prism"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		buildoptions { "/utf-8" }

		defines
		{
			PR_PLATFORM_WINDOWS
		}

	filter "configurations:Debug"
		defines "PR_DEBUG"
		optimize "Off"    
		symbols "On"
		runtime "Debug"
		links
		{
			"Prism/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
		}
		postbuildcommands
		{
			("{COPY} ../Prism/vendor/assimp/bin/Debug/ \"../bin/" .. outputdir .. "/%{prj.name}/\""),
		}

	filter "configurations:Release"
		defines "PR_RELEASE"
		optimize "On"
		runtime "Release"
		links
		{
			"Prism/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}
		postbuildcommands
		{
			("{COPY} ../Prism/vendor/assimp/bin/Release/ \"../bin/" .. outputdir .. "/%{prj.name}/\""),
		}

	filter "configurations:Dist"
		defines "PR_DIST"
		optimize "On"
		runtime  "Release"
		links
		{
			"Prism/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}
		postbuildcommands
		{
			("{COPY} ../Prism/vendor/assimp/bin/Release/ \"../bin/" .. outputdir .. "/%{prj.name}/\""),
		}