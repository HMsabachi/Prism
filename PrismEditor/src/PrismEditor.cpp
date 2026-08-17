#include <Prism.h>
#include "EditorLayer.h"

#include "Prism/Core/EntryPoint.h"
#include "CLI/CLI11.hpp"

class PrismEditorApplication : public Prism::Application
{
public:
    PrismEditorApplication(const Prism::ApplicationProps& props)
        : Application(props)
    {
    }

    virtual void OnInit() override
    {
        Application::OnInit();
        PushLayer(new Prism::EditorLayer());
    }
};

Prism::Application* Prism::CreateApplication(int argc, char** argv)
{
    CLI::App app{ "Prism Engine / Prism引擎" };
    bool single = false;

    app.add_flag("-s,--singleThreaded", single, "singleThreaded / 单线程渲染");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
         app.exit(e);
    }
    Prism::ApplicationProps Props;
    Props.Name = "PrismEditor";
    Props.WindowWidth = 1920;
    Props.WindowHeight = 1080;
    Props.VSync = false;
    Props.CoreThreadingPolicy = single ? Prism::ThreadingPolicy::SingleThreaded : Prism::ThreadingPolicy::MultiThreaded;
    return new PrismEditorApplication(Props);
}
