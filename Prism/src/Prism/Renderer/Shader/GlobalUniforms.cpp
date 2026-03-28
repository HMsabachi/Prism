#include "prpch.h"
#include "GlobalUniforms.h"
#include "Platform/OpenGL/OpenGLGlobalUniform.h"
#include "../Renderer.h"


namespace Prism
{
	GlobalUniforms* GlobalUniforms::s_Instance = nullptr;
	void GlobalUniforms::Init()
	{
		s_Instance = new OpenGLGlobalUniform();
	}
}