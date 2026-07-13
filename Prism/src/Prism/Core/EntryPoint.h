#pragma once

#ifdef PR_PLATFORM_WINDOWS

extern Prism::Application* Prism::CreateApplication();

namespace Prism { extern PRISM_API bool g_ApplicationRunning; }

int main(int argc, char** argv)
{
    while (Prism::g_ApplicationRunning)
    {
        PR_PROFILE_BEGIN_SESSION("Startup", "PrismProfile-Startup.json");
        auto app = Prism::CreateApplication();
        PR_CORE_ASSERT(app, "Client Application is null!");
        PR_PROFILE_END_SESSION();

        PR_PROFILE_BEGIN_SESSION("Runtime", "PrismProfile-Runtime.json");
        app->Run();
        PR_PROFILE_END_SESSION();

        PR_PROFILE_BEGIN_SESSION("Shutdown", "PrismProfile-Shutdown.json");
        delete app;
        PR_PROFILE_END_SESSION();
    }
    return 0;
}

#endif
