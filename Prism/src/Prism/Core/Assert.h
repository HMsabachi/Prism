#pragma once

#ifdef PR_DEBUG
#define PR_ENABLE_ASSERTS
#endif

#ifdef PR_ENABLE_ASSERTS
#define PR_ASSERT_NO_MESSAGE(condition) { if(!(condition)) { PR_ERROR("Assertion Failed"); __debugbreak(); } }
#define PR_ASSERT_MESSAGE(condition, ...) { if(!(condition)) { PR_ERROR(__VA_ARGS__); __debugbreak(); } }

#define PR_ASSERT_RESOLVE(_1, _2, _3, _4, _5, _6, _7, macro, ...) macro
#define PR_GET_ASSERT_MACRO(...) PR_EXPAND_VARGS(PR_ASSERT_RESOLVE(__VA_ARGS__, PR_ASSERT_MESSAGE, PR_ASSERT_MESSAGE, PR_ASSERT_MESSAGE, PR_ASSERT_MESSAGE, PR_ASSERT_MESSAGE, PR_ASSERT_MESSAGE, PR_ASSERT_NO_MESSAGE))

#define PR_ASSERT(...) PR_EXPAND_VARGS( PR_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define PR_CORE_ASSERT(...) PR_EXPAND_VARGS( PR_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
#define PR_ASSERT(...)
#define PR_CORE_ASSERT(...)
#endif