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

editandcontinue "Off"

-- Vulkan SDK: read from env into a Lua global so %{VULKAN_SDK} token expansion works in IncludeDir/LibraryDir below.
VULKAN_SDK = os.getenv("VULKAN_SDK")

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
IncludeDir["PhysX"] = "Prism/vendor/PhysX/include"
IncludeDir["PrismShaderCore"] = "Prism/vendor/PrismShaderCompiler/PrismShaderCore/include/"
IncludeDir["Vulkan"] = "%{VULKAN_SDK}/Include"
IncludeDir["Python"] = "vendor/Python/include/Python"
IncludeDir["pybind11"] = "Prism/vendor/pybind11/include"
IncludeDir["CLI11"] = "Prism/vendor/CLI11/Include"
IncludeDir["Tracy"] = "Prism/vendor/tracy/tracy/public"

LibraryDir = {}
LibraryDir["nethost"] = "Prism/vendor/nethost"
LibraryDir["PhysX"] = "Prism/vendor/PhysX/lib"
LibraryDir["Vulkan"] = "%{VULKAN_SDK}/Lib"
group "Dependencies"
    include "Prism/vendor/GLFW"
    include "Prism/vendor/Glad"
    include "Prism/vendor/imgui"
    include "Prism/vendor/Rolky/Rolky.Native"
    include "Prism/vendor/box2d"
    include "Prism/vendor/PrismShaderCompiler/PrismShaderCore"
    include "Prism/vendor/tracy"
group ""

group "Core"

project "Prism"
    location "Prism"
    kind "SharedLib"
    language "C++"
    staticruntime "off"

    defines { "PR_DYNAMIC_LINK" , "_CRT_SECURE_NO_WARNINGS", "yaml_cpp_EXPORTS", "TRACY_ENABLE", "TRACY_ON_DEMAND", "GLM_FORCE_DEPTH_ZERO_TO_ONE"}

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
        "%{prj.name}/vendor/yaml-cpp/include/**.h",
        "%{prj.name}/vendor/VulkanMemoryAllocator/**.cpp",
        "%{prj.name}/vendor/VulkanMemoryAllocator/**.h"
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
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.PhysX}",
        "%{IncludeDir.Python}",
        "%{IncludeDir.pybind11}",
        "%{IncludeDir.PrismShaderCore}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.Tracy}"
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
        "PrismShaderCore",
        "Tracy",
        "opengl32.lib",
        "dwmapi.lib",
        "vulkan-1.lib"
    }
    linkoptions { "/WHOLEARCHIVE:ImGui" }
    libdirs
    {
        "%{IncludeDir.nethost}",
        "%{LibraryDir.PhysX}/%{cfg.buildcfg}",
        "%{LibraryDir.Vulkan}",
        "vendor/Python/libs"
    }
    includedirs { "%{prj.name}/src/Scripting" }
    links { "nethost" }

    filter "files:%{prj.name}/src/Scripting/CSharp/Native/**.cpp or files:Prism/vendor/yaml-cpp/src/**.cpp"
        flags { "NoPCH" }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

        buildoptions { "/utf-8", "/bigobj" }

        defines
        {
            "PR_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/PrismEditor/\""),
            ('{COPY} "vendor/nethost/*.dll" "../bin/%{outputdir}/PrismEditor/"'),
            ('{COPY} "vendor/PhysX/bin/%{cfg.buildcfg}/*.dll" "../bin/%{outputdir}/PrismEditor/"'),
            ('{COPY} "../vendor/Python/python313.dll" "../bin/%{outputdir}/PrismEditor/"'),
            ('{COPY} "../vendor/Python/python3.dll" "../bin/%{outputdir}/PrismEditor/"'),
            ('{COPY} "../vendor/Python/vcruntime140.dll" "../bin/%{outputdir}/PrismEditor/"'),
            ('{COPY} "../vendor/Python/vcruntime140_1.dll" "../bin/%{outputdir}/PrismEditor/"'),
            ('{COPYDIR} "../vendor/Python/Lib" "../bin/%{outputdir}/PrismEditor/Lib"'),
            ('{COPYDIR} "../vendor/Python/DLLs" "../bin/%{outputdir}/PrismEditor/DLLs"'),
        }
    filter "configurations:Debug"
        defines  "PR_DEBUG"
        optimize "Off"
        symbols "On"
        runtime "Debug"
        links
        {
            "Prism/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib",
            "PhysX_64.lib",
            "PhysXCommon_64.lib",
            "PhysXCooking_64.lib",
            "PhysXFoundation_64.lib",
            "PhysXExtensions_static_64.lib",
            "PhysXPvdSDK_static_64.lib"
        }
        

    filter "configurations:Release"
        defines {"PR_RELEASE", "NDEBUG"}
        optimize "On"
        runtime "Release"
        links
        {
            "Prism/vendor/assimp/bin/Release/assimp-vc141-mt.lib",
            "PhysX_64.lib",
            "PhysXCommon_64.lib",
            "PhysXCooking_64.lib",
            "PhysXFoundation_64.lib",
            "PhysXExtensions_static_64.lib",
            "PhysXPvdSDK_static_64.lib"
        }


    filter "configurations:Dist"
        defines {"PR_DIST", "NDEBUG"}
        optimize "On"
        runtime "Release"
        links
        {
            "Prism/vendor/assimp/bin/Release/assimp-vc141-mt.lib",
            "PhysX_64.lib",
            "PhysXCommon_64.lib",
            "PhysXCooking_64.lib",
            "PhysXFoundation_64.lib",
            "PhysXExtensions_static_64.lib",
            "PhysXPvdSDK_static_64.lib"
        }

project "PrismEditor"
    location "PrismEditor"
    kind "ConsoleApp"
    language "C++"
    staticruntime "off"

    defines { "PR_DYNAMIC_LINK", "TRACY_ENABLE", "TRACY_ON_DEMAND", "GLM_FORCE_DEPTH_ZERO_TO_ONE" }

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
        "%{IncludeDir.entt}",
        "%{IncludeDir.Rolky}",
        "%{IncludeDir.Python}",
        "%{IncludeDir.PrismShaderCore}",
        "%{IncludeDir.pybind11}",
        "%{IncludeDir.CLI11}",
        "%{IncludeDir.Tracy}"
    }

    links
    {
        "Prism"      
    }

    filter "system:windows"
        cppdialect "C++20"
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
    targetdir ("PrismEditor/Assets/Scripts/net9.0")
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
            "CS8500", "CS8618"
        }

    files 
    {
        "%{prj.name}/src/**.cs", 
    }

    links
    {
        "Prism.Scripting"
    }

    filter "configurations:Release"
        local ScriptSrc = path.getabsolute("ExampleApp/src")
        postbuildcommands
        {
            '{COPYDIR} "' .. ScriptSrc .. '" "%{wks.location}/bin/Release-windows-x86_64/PrismEditor/Assets/Scripts/CSharp/src"'
        }
group ""
