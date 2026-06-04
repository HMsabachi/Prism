project "Rolky.Managed"
    filter { "not action:vs*", "not system:windows" }
        kind "StaticLib"
		-- Mach-y AR requires a non-empty file list for archive creation
		files { "Source/Dummy.cpp" }

    filter { "action:vs* or system:windows" }
        language "C#"
        dotnetframework "net9.0"
        kind "SharedLib"
        clr "Unsafe"
        targetdir("../Build/%{cfg.buildcfg}")
        objdir("../Intermediates/%{cfg.buildcfg}")
        dependson { "Rolky.Generator" }

		vsprops {
			AppendTargetFrameworkToOutputPath = "false",
			Nullable = "enable",
			CopyLocalLockFileAssemblies = "true",
			EnableDynamicLoading = "true",
		}

        disablewarnings {
            "CS8500"
        }

        files {
            "Source/**.cs"
        }

        links {
            "../vendor/Roslyn/Microsoft.CodeAnalysis.dll",
            "../vendor/Roslyn/Microsoft.CodeAnalysis.CSharp.dll",
            "../vendor/Roslyn/Microsoft.CodeAnalysis.Analyzers.dll",
            "../vendor/Roslyn/Microsoft.CodeAnalysis.CSharp.Analyzers.dll",
        }
