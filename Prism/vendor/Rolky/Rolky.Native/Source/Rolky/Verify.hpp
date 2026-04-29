#pragma once

#define ROLKY_SOURCE_LOCATION const char* file = __FILE__; int line = __LINE__

#if defined(__GNUC__)
	#define ROLKY_DEBUG_BREAK __builtin_trap()
#elif defined(_MSC_VER)
	#define ROLKY_DEBUG_BREAK __debugbreak()
#else
	#define ROLKY_DEBUG_BREAK	
#endif

#define ROLKY_VERIFY(expr) do {\
						if(!(expr))\
						{\
							ROLKY_SOURCE_LOCATION;\
							std::cerr << "[Rolky.Native]: Assert Failed! Expression: " << #expr << " at " << file << ":" << line << "\n";\
							ROLKY_DEBUG_BREAK;\
						}\
					} while(0)
