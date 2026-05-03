workspace "Prism"
	architecture "x86_64"
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
IncludeDir["AllVendor"] = "Prism/vendor"
IncludeDir["GLFW"] = "Prism/vendor/GLFW/include"
IncludeDir["Glad"] = "Prism/vendor/Glad/include"
IncludeDir["ImGui"] = "Prism/vendor/imgui"
IncludeDir["glm"] = "Prism/vendor/glm"
IncludeDir["stb_image"] = "Prism/vendor/stb_image"
IncludeDir["nethost"] = "Prism/vendor/nethost"
IncludeDir["entt"] = "Prism/vendor/entt/include"
IncludeDir["FastNoise"] = "Prism/vendor/FastNoise"
IncludeDir["Rolky"] = "Prism/vendor/Rolky/Rolky.Native/Include"
IncludeDir["yaml"] = "Prism/vendor/yaml-cpp/include"
IncludeDir["Box2D"] = "Prism/vendor/box2d/include"

LibraryDir = {}
LibraryDir["nethost"] = "Prism/vendor/nethost"
group "Dependencies"
	include "Prism/vendor/GLFW"
	include "Prism/vendor/Glad"
	include "Prism/vendor/imgui"
	include "Prism/vendor/Rolky/Rolky.Native"
	include "Prism/vendor/box2d"
group ""

group "Core"

project "Prism"
	location "Prism"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

	defines { "PR_DYNAMIC_LINK" , "_CRT_SECURE_NO_WARNINGS", "yaml_cpp_EXPORTS"}

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
		"%{prj.name}/vendor/glm/glm/**.inl",
        "%{prj.name}/vendor/yaml-cpp/src/**.cpp",
		"%{prj.name}/vendor/yaml-cpp/src/**.h",
		"%{prj.name}/vendor/yaml-cpp/include/**.h"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.AllVendor}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
			"%{prj.name}/vendor/assimp/include",
		"%{IncludeDir.nethost}",
		"%{IncludeDir.entt}",
        "%{IncludeDir.FastNoise}",
		"%{IncludeDir.Rolky}",
        "%{IncludeDir.yaml}",
        "%{IncludeDir.Box2D}"
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
		"Rolky.Native",
		"Box2D",
		"opengl32.lib",
		"dwmapi.lib"
	}
	linkoptions { "/WHOLEARCHIVE:ImGui" }
	libdirs
	{
		"%{IncludeDir.nethost}"      
	}
	includedirs { "%{prj.name}/src/Scripting" }
	links { "nethost" }   

	filter "files:%{prj.name}/src/Scripting/Native/**.cpp or files:Prism/vendor/yaml-cpp/src/**.cpp"
   		flags { "NoPCH" }

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
			"%{IncludeDir.nethost}",
		"%{IncludeDir.entt}"

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

group ""
workspace "PrismManaged"
configurations
{
	"Debug",
	"Release",
	"Dist"
}
include "Prism/vendor/Rolky/Rolky.Managed"
project "Prism.Scripting"
	location "Prism.Scripting"
	kind "SharedLib"
	language "C#"
	dotnetframework "net9.0"          -- 使用 .NET 9

	filter { "action:vs* or system:windows" }
        language "C#"
        clr "Unsafe"
		vsprops {
			AppendTargetFrameworkToOutputPath = "false",
			Nullable = "enable",
			CopyLocalLockFileAssemblies = "true",
			EnableDynamicLoading = "true",
		}
	disablewarnings {
            "CS8500"
        }

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	links {
		"Rolky.Managed"
	}

	files
	{
		"%{prj.name}/src/**.cs",       
	}

group "Examples"
project "ExampleApp"
	location "ExampleApp"
	dotnetframework "net9.0"      
	kind "SharedLib"
	language "C#"
	clr "Unsafe"
	targetdir ("PrismEditor/Assets/Scripts")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	filter { "action:vs* or system:windows" }
        language "C#"
        clr "Unsafe"
		vsprops {
			AppendTargetFrameworkToOutputPath = "false",
			Nullable = "enable",
			CopyLocalLockFileAssemblies = "true",
			EnableDynamicLoading = "true",
		}
	disablewarnings {
            "CS8500"
        }

	files 
	{
		"%{prj.name}/src/**.cs", 
	}

	links
	{
		"Prism.Scripting"
	}
group ""