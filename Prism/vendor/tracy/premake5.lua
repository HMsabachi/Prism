project "Tracy"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    warnings "Off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "TRACY_ENABLE",
        "TRACY_ON_DEMAND"
    }

    files
    {
        "tracy/public/TracyClient.cpp"
    }

    includedirs
    {
        "tracy/public"
    }

    filter "system:windows"
        systemversion "latest"
        buildoptions { "/bigobj" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
        symbols "off"
