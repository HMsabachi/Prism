#pragma once

namespace Prism {

	class PRISM_API PhysicsSettingsWindow
	{
	public:
		static void OnImGuiRender(bool* show);

	private:
		static void RenderWorldSettings();
		static void RenderLayerList();
		static void RenderSelectedLayer();

	private:
		static bool Property(const char* label, float& value, float min = -1.0F, float max = 1.0F);
	};

}
