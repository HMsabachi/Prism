-- vendor/PrismShaderParser/premake5.lua
-- PrismShaderParser 独立静态库
-- 风格严格保持 Prism 引擎一致：中文注释、%{wks.location}、outputdir 等

project "PrismShaderParser"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    defines { "PR_DYNAMIC_LINK" , "_CRT_SECURE_NO_WARNINGS" }

    -- 输出目录与主项目保持一致
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- 文件列表（按你的仓库风格组织）
    files
    {
        "include/**.h",
        "src/**.h",
        "src/**.cpp"
    }


    -- 定义（可根据需要加）
    defines
    {
        "PRISM_SHADER_PARSER_STATIC"         -- 可选，用于区分静态/动态
    }

    -- 平台过滤
    filter "system:windows"
        systemversion "latest"
        defines { "PR_PLATFORM_WINDOWS", "PR_BUILD_DLL" }

    filter "configurations:Debug"
        defines { "PR_DEBUG" }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines { "PR_RELEASE" }
        runtime "Release"
        optimize "on"
