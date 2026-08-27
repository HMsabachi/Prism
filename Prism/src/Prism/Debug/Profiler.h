#pragma once

#if !defined(PR_DIST)
    #define PR_ENABLE_PROFILING 1
#else
    #define PR_ENABLE_PROFILING 0
#endif

#if PR_ENABLE_PROFILING
    #include <tracy/Tracy.hpp>
#endif

#if PR_ENABLE_PROFILING
    #define PR_PROFILE_MARK_FRAME           FrameMark;
    #define PR_PROFILE_FUNCTION()           ZoneScoped
    #define PR_PROFILE_SCOPE(name)          ZoneScopedN(name)
    #define PR_PROFILE_THREAD(...)          tracy::SetThreadName(__VA_ARGS__)
    #define PR_PROFILE_PLOT(name, value)    TracyPlot(name, value)
#else
    #define PR_PROFILE_MARK_FRAME
    #define PR_PROFILE_FUNCTION()
    #define PR_PROFILE_SCOPE(name)
    #define PR_PROFILE_THREAD(...)
    #define PR_PROFILE_PLOT(name, value)
#endif
