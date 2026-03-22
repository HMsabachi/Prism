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

#ifdef PR_ENABLE_ASSERTS
#define PR_ASSERT(x, ...) { if(!(x)) { PR_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define PR_CORE_ASSERT(x, ...) { if(!(x)) { PR_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define PR_ASSERT(x, ...)
#define PR_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
#define PR_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)