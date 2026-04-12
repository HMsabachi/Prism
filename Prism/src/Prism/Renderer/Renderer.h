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

		static void* Submit(RenderCommandFn fn, unsigned int size)
		{
			return s_Instance->m_CommandQueue.Allocate(fn, size);
		}
		static void WaitAndRender();

		inline static Renderer& Get() { return *s_Instance; }

		static void BeginScene(Camera& camera) { s_Instance->IBeginScene(camera); }
		static void EndScene() { s_Instance->IEndScene(); }

		// ~Actual~ Renderer here... TODO: remove confusion later
		static void BeginRenderPass(const Ref<RenderPass>& renderPass) { s_Instance->IBeginRenderPass(renderPass); }
		static void EndRenderPass() { s_Instance->IEndRenderPass(); }

		static void SubmitMesh(const Ref<Mesh>& mesh) { s_Instance->SubmitMeshI(mesh); }

	private:
		void IBeginScene(const Camera& camera);
		void IEndScene();
		void IBeginRenderPass(const Ref<RenderPass>& renderPass);
		void IEndRenderPass();

		void SubmitMeshI(const Ref<Mesh>& mesh);

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


#define PR_RENDER_PASTE2(a, b) a ## b
#define PR_RENDER_PASTE(a, b) PR_RENDER_PASTE2(a, b)
#define PR_RENDER_UNIQUE(x) PR_RENDER_PASTE(x, __LINE__)

#define PR_RENDER(code) \
    struct PR_RENDER_UNIQUE(PRRenderCommand) \
    {\
        static void Execute(void*)\
        {\
            code\
        }\
    };\
	{\
		auto mem = ::Prism::Renderer::Submit(PR_RENDER_UNIQUE(PRRenderCommand)::Execute, sizeof(PR_RENDER_UNIQUE(PRRenderCommand)));\
		new (mem) PR_RENDER_UNIQUE(PRRenderCommand)();\
	}\

#define PR_RENDER_1(arg0, code) \
	do{\
    struct PR_RENDER_UNIQUE(PRRenderCommand) \
    {\
		PR_RENDER_UNIQUE(PRRenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0) \
		: arg0(arg0) {}\
		\
        static void Execute(void* argBuffer)\
        {\
			auto& arg0 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg0;\
            code\
        }\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
    };\
	{\
		auto mem = ::Prism::Renderer::Submit(PR_RENDER_UNIQUE(PRRenderCommand)::Execute, sizeof(PR_RENDER_UNIQUE(PRRenderCommand)));\
		new (mem) PR_RENDER_UNIQUE(PRRenderCommand)(arg0);\
	}} while(0)

#define PR_RENDER_2(arg0, arg1, code) \
    struct PR_RENDER_UNIQUE(PRRenderCommand) \
    {\
		PR_RENDER_UNIQUE(PRRenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1) \
		: arg0(arg0), arg1(arg1) {}\
		\
        static void Execute(void* argBuffer)\
        {\
			auto& arg0 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg1;\
            code\
        }\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
    };\
	{\
		auto mem = ::Prism::Renderer::Submit(PR_RENDER_UNIQUE(PRRenderCommand)::Execute, sizeof(PR_RENDER_UNIQUE(PRRenderCommand)));\
		new (mem) PR_RENDER_UNIQUE(PRRenderCommand)(arg0, arg1);\
	}\

#define PR_RENDER_3(arg0, arg1, arg2, code) \
    struct PR_RENDER_UNIQUE(PRRenderCommand) \
    {\
		PR_RENDER_UNIQUE(PRRenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2) \
		: arg0(arg0), arg1(arg1), arg2(arg2) {}\
		\
        static void Execute(void* argBuffer)\
        {\
			auto& arg0 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg1;\
			auto& arg2 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg2;\
            code\
        }\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2;\
    };\
	{\
		auto mem = ::Prism::Renderer::Submit(PR_RENDER_UNIQUE(PRRenderCommand)::Execute, sizeof(PR_RENDER_UNIQUE(PRRenderCommand)));\
		new (mem) PR_RENDER_UNIQUE(PRRenderCommand)(arg0, arg1, arg2);\
	}\

#define PR_RENDER_4(arg0, arg1, arg2, arg3, code) \
    struct PR_RENDER_UNIQUE(PRRenderCommand) \
    {\
		PR_RENDER_UNIQUE(PRRenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3)\
		: arg0(arg0), arg1(arg1), arg2(arg2), arg3(arg3) {}\
		\
        static void Execute(void* argBuffer)\
        {\
			auto& arg0 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg1;\
			auto& arg2 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg2;\
			auto& arg3 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)argBuffer)->arg3;\
            code\
        }\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3;\
    };\
	{\
		auto mem = Renderer::Submit(PR_RENDER_UNIQUE(PRRenderCommand)::Execute, sizeof(PR_RENDER_UNIQUE(PRRenderCommand)));\
		new (mem) PR_RENDER_UNIQUE(PRRenderCommand)(arg0, arg1, arg2, arg3);\
	}

#define PR_RENDER_S(code) auto self = this;\
	PR_RENDER_1(self, code)

#define PR_RENDER_S1(arg0, code) auto self = this;\
	PR_RENDER_2(self, arg0, code)

#define PR_RENDER_S2(arg0, arg1, code) auto self = this;\
	PR_RENDER_3(self, arg0, arg1, code)

#define PR_RENDER_S3(arg0, arg1, arg2, code) auto self = this;\
	PR_RENDER_4(self, arg0, arg1, arg2, code)