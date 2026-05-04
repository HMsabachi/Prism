#include "prpch.h"
#include "Pipeline.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLPipeline.h"

namespace Prism {

	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLPipeline>::Create(spec);
		}
		PR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
