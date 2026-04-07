#pragma once
#include <memory>

#ifdef _WIN32
	#ifdef _WIN64
		#define PR_PLATFORM_WINDOWS
	#else
		#error Prism only supports 64-bit Windows!
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		#error IOS simulator is not supported!
	#elif TARGET_OS_IPHONE == 1
		#define PR_PLATFORM_IOS
		#error IOS is not supported!
	#elif TARGET_OS_MAC == 1
		#define PR_PLATFORM_MACOS
		#error MacOS is not supported!
	#else
		#error Unknown Apple platform!
	#endif
#elif defined(__ANDROID__)
	#define PR_PLATFORM_ANDROID
	#error Android is not supported!
#elif defined(__linux__)
	#define PR_PLATFORM_LINUX
	#error Linux is not supported!
#else
	#error Unknown Platform!
#endif

#ifdef PR_PLATFORM_WINDOWS
	#if PR_DYNAMIC_LINK
		#ifdef PR_BUILD_DLL
			#define PRISM_API __declspec(dllexport)
		#else
			#define PRISM_API __declspec(dllimport)
		#endif
	#else
		#define PRISM_API
	#endif
#else
	#error Prism only supports Windows!
#endif

#ifdef PR_DEBUG
	#define PR_ENABLE_ASSERTS
#endif
#ifdef PR_ENABLE_ASSERTS
	#define PR_ASSERT_NO_MESSAGE(condition) { if(!(condition)) { PR_ERROR("Assertion Failed!"); __debugbreak(); } }
	#define PR_ASSERT_MESSAGE(condition, ...) { if(!(condition)) { PR_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }

	#define PR_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro

	#define PR_ASSERT(...) PR_ASSERT_RESOLVE(__VA_ARGS__, PR_ASSERT_MESSAGE, PR_ASSERT_NO_MESSAGE)(__VA_ARGS__)
	#define PR_CORE_ASSERT(...) PR_ASSERT_RESOLVE(__VA_ARGS__, PR_ASSERT_MESSAGE, PR_ASSERT_NO_MESSAGE)(__VA_ARGS__)
#else
	#define PR_ASSERT(...)
	#define PR_CORE_ASSERT(...)
#endif

#define BIT(x) (1 << x)
#define PR_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Prism
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	using byte = unsigned char;

}