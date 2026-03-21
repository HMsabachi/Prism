#pragma once

#ifdef PR_PLATFORM_WINDOWS
	#ifdef PR_BUILD_DLL
		#define PRISM_API __declspec(dllexport)
	#else
		#define PRISM_API __declspec(dllimport)
	#endif
#else
	#error Prism only supports Windows!
#endif

#define BIT(x) (1 << x)