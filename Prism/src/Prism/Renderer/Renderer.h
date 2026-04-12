#pragma once

#include "RendererTypes.h"
#include "RenderCommandQueue.h"
#include "RendererAPI.h"

#include "RenderPass.h"

#include "Mesh.h"
#include "Shader/GlobalUniforms.h"

namespace Prism
{
	class ShaderLibrary;
	class Camera;
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
		static void Init();

		static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);
		static void SetClearColor(float r, float g, float b, float a);
		static void ClearMagenta();

		static void DrawIndexed(uint32_t count, bool depthTest = true);

		static const Scope<ShaderLibrary>& GetShaderLibrary();

		template<typename FuncT>
		static void Submit(FuncT&& func)
		{
			auto renderCmd = [](void* ptr) {
				auto pFunc = (FuncT*)ptr;
				(*pFunc)();
				pFunc->~FuncT();
				};
			auto storageBuffer = s_Instance->m_CommandQueue.Allocate(renderCmd, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}
		/*static void* Submit(RenderCommandFn fn, unsigned int size)
		{
			return s_Instance->m_CommandQueue.Allocate(fn, size);
		}*/

		static void WaitAndRender();

		inline static Renderer& Get() { return *s_Instance; }

		static void BeginScene(Camera& camera) { s_Instance->IBeginScene(camera); }
		static void EndScene() { s_Instance->IEndScene(); }

		// ~Actual~ Renderer here... TODO: remove confusion later
		static void BeginRenderPass(const Ref<RenderPass>& renderPass) { s_Instance->IBeginRenderPass(renderPass); }
		static void EndRenderPass() { s_Instance->IEndRenderPass(); }

		static void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialInstance>& overrideMaterial = nullptr) { s_Instance->SubmitMeshI(mesh, transform, overrideMaterial); }

	private:
		void IBeginScene(const Camera& camera);
		void IEndScene();
		void IBeginRenderPass(const Ref<RenderPass>& renderPass);
		void IEndRenderPass();

		void SubmitMeshI(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialInstance>& overrideMaterial);

		void UpdateGlobalsUBO(const Camera& camera);
	private:
		static Renderer* s_Instance;

	private:
		PrismGlobalsUBO m_GlobalsUBO;
		const Camera* m_ActiveCamera;

		Ref<RenderPass> m_ActiveRenderPass;
		RenderCommandQueue m_CommandQueue;

		Scope<ShaderLibrary> m_ShaderLibrary;
	};
}