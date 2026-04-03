#pragma once

#include "RenderCommandQueue.h"
#include "RendererAPI.h"



namespace Prism
{
	class PRISM_API Renderer
	{
	private:

	public:
		typedef void(*RenderCommandFn)(void*);

		static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);
		static void SetClearColor(float r, float g, float b, float a);

		static void ClearMagenta();

		void Init();

		static void* Submit(RenderCommandFn fn, unsigned int size)
		{
			return s_Instance->m_CommandQueue.Allocate(fn, size);
		}
		static void WaitAndRender();

		inline static Renderer& Get() { return *s_Instance; }

	private:
		static Renderer* s_Instance;

		RenderCommandQueue m_CommandQueue;
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
		auto mem = RenderCommandQueue::Submit(sizeof(PR_RENDER_UNIQUE(PRRenderCommand)), PR_RENDER_UNIQUE(PRRenderCommand)::Execute);\
		new (mem) PR_RENDER_UNIQUE(PRRenderCommand)();\
	}\

#define PR_RENDER_1(arg0, code) \
	do{\
    struct PR_RENDER_UNIQUE(PRRenderCommand) \
    {\
		PR_RENDER_UNIQUE(PRRenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0) \
		: arg0(arg0) {}\
		\
        static void Execute(void* self)\
        {\
			auto& arg0 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg0;\
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
        static void Execute(void* self)\
        {\
			auto& arg0 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg0;\
			auto& arg1 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg1;\
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
        static void Execute(void* self)\
        {\
			auto& arg0 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg0;\
			auto& arg1 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg1;\
			auto& arg2 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg2;\
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
        static void Execute(void* self)\
        {\
			auto& arg0 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg0;\
			auto& arg1 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg1;\
			auto& arg2 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg2;\
			auto& arg3 = ((PR_RENDER_UNIQUE(PRRenderCommand)*)self)->arg3;\
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