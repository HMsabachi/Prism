#pragma once

#include "Prism/Renderer/Texture.h"

namespace Prism {

	class PRISM_API ObjectsPanel
	{
	public:
		ObjectsPanel();

		void OnImGuiRender();

	private:
		Ref<Texture2D> m_CubeImage;
	};

}
