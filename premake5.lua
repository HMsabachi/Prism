workspace "Prism"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
-- 包括相对于根文件夹（解决方案目录）的子目录
IncludeDir = {}
IncludeDir["GLFW"] = "Prism/vendor/GLFW/include"
IncludeDir["Glad"] = "Prism/vendor/Glad/include"
IncludeDir["ImGui"] = "Prism/vendor/imgui"
IncludeDir["glm"] = "Prism/vendor/glm"
IncludeDir["stb_image"] = "Prism/vendor/stb_image"
IncludeDir["PrismShaderParser"] = "Prism/vendor/PrismShaderParser/src"

group "Dependencies"
	include "Prism/vendor/GLFW"
	include "Prism/vendor/Glad"
	include "Prism/vendor/imgui"
	include "Prism/vendor/PrismShaderParser"
group ""

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
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\""),
			("{COPY} %{prj.location}/vendor/assimp/win64/ \"../bin/" .. outputdir .. "/Sandbox/\"")
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
			("{COPY} %{prj.location}/vendor/assimp/bin/Debug/ \"../bin/" .. outputdir .. "/Sandbox/\""),
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
			("{COPY} %{prj.location}/vendor/assimp/bin/Release/ \"../bin/" .. outputdir .. "/Sandbox/\""),
		}


	filter "configurations:Dist"
		defines "PR_DIST"
		optimize "On"
		runtime "Release"
		links
		{
			"Prism/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}
		postbuildcommands
		{
			("{COPY} %{prj.location}/vendor/assimp/bin/Release/ \"../bin/" .. outputdir .. "/Sandbox/\""),
		}

project "Sandbox"
	location "Sandbox"
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
		"%{IncludeDir.PrismShaderParser}"

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
		runtime  "Release"
		links
		{
			"Prism/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}