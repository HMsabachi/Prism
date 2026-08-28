#include <Prism.h>
#include "EditorLayer.h"

#include "Prism/Core/EntryPoint.h"
#include "CLI/CLI11.hpp"
#include <map>

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
    bool noVSync = false;
#ifdef PR_DEBUG
    Prism::RendererAPIType renderer = Prism::RendererAPIType::Vulkan;
#else
    Prism::RendererAPIType renderer = Prism::RendererAPIType::OpenGL;
#endif
    // renderer = Prism::RendererAPIType::OpenGL;
    renderer = Prism::RendererAPIType::Vulkan;

    app.add_flag("-s,--singleThreaded", single, "singleThreaded / 单线程渲染");
    app.add_flag("--no-vsync", noVSync, "disable VSync / 关闭垂直同步");
    app.add_option("-r,--renderer", renderer, "Renderer API: opengl | vulkan / 渲染后端")
        ->transform(CLI::CheckedTransformer(
            std::map<std::string, Prism::RendererAPIType>{
                { "opengl", Prism::RendererAPIType::OpenGL },
                { "vulkan", Prism::RendererAPIType::Vulkan }
            }, CLI::ignore_case));

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
    Props.VSync = true;
    Props.CoreThreadingPolicy = single ? Prism::ThreadingPolicy::SingleThreaded : Prism::ThreadingPolicy::MultiThreaded;
    Props.RendererAPI = renderer;
    return new PrismEditorApplication(Props);
}
