include "../Premake/DebuggerTypeExtension.lua"

project "Rolky.Native"
    language "C++"
    cppdialect "C++17"
    kind "StaticLib"
    staticruntime "Off"
    debuggertype "NativeWithManagedCore"

	dependson "Rolky.Managed"

	targetdir("../Build/%{cfg.buildcfg}")
	objdir("../Intermediates/%{cfg.buildcfg}")

    pchheader "RolkyPCH.hpp"
    pchsource "Source/RolkyPCH.cpp"

    forceincludes { "RolkyPCH.hpp" }

    filter { "action:xcode4" }
        pchheader "Source/RolkyPCH.hpp"
    filter { }

    files {
        "Source/**.cpp",
        "Source/**.hpp",

        "Include/Rolky/**.hpp",
    }

    includedirs { "Source/", "Include/" }
    externalincludedirs { "../NetCore/" }

    filter { "configurations:Debug" }
        runtime "Debug"
        symbols "On"
    filter { }

    filter { "configurations:Release" }
        runtime "Release"
        symbols "Off"
        optimize "On"
    filter { }

	filter { "system:windows" }
		defines { "ROLKY_WINDOWS" }
    filter { }

	filter { "system:macosx" }
		defines { "ROLKY_MACOSX" }
    filter { }
