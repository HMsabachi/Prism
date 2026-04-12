#pragma once

#include "Prism/Core/Core.h"

#include "Framebuffer.h"

namespace Prism {

	struct PRISM_API RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
	};

	class PRISM_API RenderPass
	{
	public:
		virtual ~RenderPass() {}

		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);
	};

}