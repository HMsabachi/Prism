#pragma once

#include "RendererTypes.h"
#include "RenderCommandQueue.h"
#include "RenderPass.h"

#include "Mesh.h"

namespace Prism
{
	class ShaderLibrary;
	class Camera;
	class MaterialInstance;
	class Mesh;
}

namespace Prism
{
	class PRISM_API Renderer
	{
	private:

	public:
		typedef void(*RenderCommandFn)(void*);
		Renderer();
		~Renderer();
		template<typename FuncT>
		static void Submit(FuncT&& func)
		{
			auto renderCmd = [](void* ptr) {
				auto pFunc = (FuncT*)ptr;
				(*pFunc)();
				pFunc->~FuncT();
				};
			auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}
		static void Init();

		static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);
		static void SetClearColor(float r, float g, float b, float a);
		static void ClearMagenta();

		static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true);

		// For OpenGL
		static void SetLineThickness(float thickness);
		static void MemoryBarriers(RendererAPI::BarrierFlags flags);

		static const Scope<ShaderLibrary>& GetShaderLibrary();

		static void BeginRenderPass(const Ref<RenderPass>& renderPass, bool clear = true);
		static void EndRenderPass();

		static void WaitAndRender();	

		static void SubmitQuad(const Ref<MaterialInstance>& material, const glm::mat4& transform = glm::mat4(1.0f));
		static void SubmitFullscreenQuad(const Ref<MaterialInstance>& material);
		static void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialInstance>& overrideMaterial = nullptr);


		static void DrawAABB(const Ref<Mesh>& mesh, const glm::vec4& color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	private:
		static RenderCommandQueue& GetRenderCommandQueue();
	};
}