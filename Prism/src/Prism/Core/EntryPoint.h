#pragma once

#ifdef PR_PLATFORM_WINDOWS

extern Prism::Application* Prism::CreateApplication(int argc, char** argv);

namespace Prism { extern PRISM_API bool g_ApplicationRunning; }

int main(int argc, char** argv)
{
    while (Prism::g_ApplicationRunning)
    {
        auto app = Prism::CreateApplication(argc, argv);
        PR_CORE_ASSERT(app, "Client Application is null!");
        app->Run();
        delete app;
    }
    return 0;
}

#endif
