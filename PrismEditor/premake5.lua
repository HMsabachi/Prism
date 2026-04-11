project "PrismEditor"
	location "%{workspace.location}/PrismEditor"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	defines "PR_DYNAMIC_LINK"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	postbuildcommands
	{
    	'{COPYDIR} "%{prj.location}/Assets" "%{cfg.targetdir}/Assets"',
	}
	
	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"../Prism/vendor/spdlog/include",
		"../Prism/src",
		"../Prism/vendor",
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