#pragma once

namespace Prism
{
	using RendererID = uint32_t;

	enum class PRISM_API RendererAPIType
	{
		None = 0,
		OpenGL = 1
	};

	struct PRISM_API RenderAPICapabilities
	{
		std::string Vendor;
		std::string Renderer;
		std::string Version;

		int MaxSamples;
		float MaxAnisotropy;

		int MaxGroupCount[3], MaxGroupSize[3], MaxInvocations;
	};

	class PRISM_API RendererAPI
	{
	public:
#pragma region 数据结构
		enum class PRISM_API Barrier
		{
			None = 0,
			ShaderStorage = BIT(0),
			VertexAttribArray = BIT(1),
			ElementArray = BIT(2),
			ImageAccess = BIT(3),
			TextureFetch = BIT(4),
			TextureUpdate = BIT(5),
			Framebuffer = BIT(6),
			Command = BIT(7),
			All = BIT(8)
		}; 
		typedef BitFlags<Barrier> BarrierFlags;
#pragma endregion
	private:

	public:
		static void Init();
		static void Shutdown();
		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void Clear(float r, float g, float b, float a);
		static void SetClearColor(float r, float g, float b, float a);
		static void DrawIndexed(unsigned int const, bool depthTest = true);
		static void MemoryBarriers(BarrierFlags flags);

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static void LoadRequiredAssets();
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};
	using MBarrier = RendererAPI::Barrier;
}