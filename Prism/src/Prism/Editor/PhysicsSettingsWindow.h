#pragma once

namespace Prism {

	class PRISM_API PhysicsSettingsWindow
	{
	public:
		static void OnImGuiRender(bool* show);
		static void RenderLayerList();
		static void RenderSelectedLayer();
	};

}
