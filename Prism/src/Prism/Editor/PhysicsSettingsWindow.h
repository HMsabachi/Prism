#pragma once

namespace Prism {

    class PRISM_API PhysicsSettingsWindow
    {
    public:
        static void OnImGuiRender(bool& show);

    private:
        static void RenderWorldSettings();
        static void RenderLayerList();
        static void RenderSelectedLayer();
    };

}
