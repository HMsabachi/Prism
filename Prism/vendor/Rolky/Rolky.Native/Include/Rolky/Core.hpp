#pragma once

#include <string_view>
#include <string>

#include <cstdint>
#include <cwchar>

#define ROLKY_DEPRECATE_MSG_P(s, x) s ". See `" x "`"

#define ROLKY_GLOBAL_ALC_MSG "Global type cache has been superseded by Assembly/ALC-local type APIs"
#define ROLKY_GLOBAL_ALC_MSG_P(x) ROLKY_DEPRECATE_MSG_P(ROLKY_GLOBAL_ALC_MSG, #x)

#define ROLKY_LEAK_UC_TYPES_MSG "Global namespace string type abstraction will be removed"
#define ROLKY_LEAK_UC_TYPES_MSG_P(x) ROLKY_DEPRECATE_MSG_P(ROLKY_LEAK_UC_TYPES_MSG, #x)

#ifdef _WIN32
	#define ROLKY_WINDOWS
#elif defined(__APPLE__)
	#define ROLKY_APPLE
#endif

#ifdef ROLKY_WINDOWS
	#define ROLKY_CALLTYPE __cdecl
	#define ROLKY_HOSTFXR_NAME "hostfxr.dll"

	// TODO(Emily): On Windows shouldn't this use the `UNICODE` macro?
	#ifdef _WCHAR_T_DEFINED
		#define ROLKY_WIDE_CHARS
	#endif
#else
	#define ROLKY_CALLTYPE

	#ifdef ROLKY_APPLE
		#define ROLKY_HOSTFXR_NAME "libhostfxr.dylib"
	#else
		#define ROLKY_HOSTFXR_NAME "libhostfxr.so"
	#endif
#endif

#ifdef ROLKY_WIDE_CHARS
	#define ROLKY_STR(s) L##s

	using CharType [[deprecated(ROLKY_LEAK_UC_TYPES_MSG_P(Rolky::UCChar))]] = wchar_t;
	using StringView [[deprecated(ROLKY_LEAK_UC_TYPES_MSG_P(Rolky::UCStringView))]] = std::wstring_view;

	namespace Rolky {
		using UCChar = wchar_t;
		using UCStringView = std::wstring_view;
		using UCString = std::wstring;
	}
#else
	#define ROLKY_STR(s) s

	using CharType [[deprecated(ROLKY_LEAK_UC_TYPES_MSG_P(Rolky::UCChar))]] = char;
	using StringView [[deprecated(ROLKY_LEAK_UC_TYPES_MSG_P(Rolky::UCStringView))]] = std::string_view;

	namespace Rolky {
		using UCChar = char;
		using UCStringView = std::string_view;
		using UCString = std::string;
	}
#endif

// TODO(Emily): Make a better system for supported version ranges (See `HostInstance.cpp:GetHostFXRPath()`)
/*
#define ROLKY_DOTNET_TARGET_VERSION_MAJOR 8
#define ROLKY_DOTNET_TARGET_VERSION_MAJOR_STR '8'
*/
#define ROLKY_UNMANAGED_CALLERS_ONLY ((const UCChar*) (-1ULL))

namespace Rolky {

	using Bool32 = uint32_t;
	static_assert(sizeof(Bool32) == 4);

	enum class TypeAccessibility
	{
		Public,
		Private,
		Protected,
		Internal,
		ProtectedPublic,
		PrivateProtected
	};

	using TypeId = int32_t;
	using ManagedHandle = int32_t;

	struct InternalCall
	{
		// TODO(Emily): Review all `UCChar*` refs to see if they could be `UCStringView`.
		const UCChar* Name;
		void* NativeFunctionPtr;
	};

}
