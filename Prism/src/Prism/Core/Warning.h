#pragma once
#ifdef _MSC_VER

#define PR_WARNING_PUSH \
    __pragma(warning(push))

#define PR_WARNING_DISABLE(x) \
    __pragma(warning(disable:x))

#define PR_WARNING_POP \
    __pragma(warning(pop))


#elif defined(__clang__) || defined(__GNUC__)

#define PR_WARNING_PUSH \
    _Pragma("GCC diagnostic push")

#define PR_WARNING_DISABLE(x) \
    _Pragma(#x)

#define PR_WARNING_POP \
    _Pragma("GCC diagnostic pop")

#endif
