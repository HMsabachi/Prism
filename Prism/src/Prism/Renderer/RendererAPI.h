#pragma once
#include "RendererTypes.h"
#include "Prism/Utilities/BitFlags.h"
#include "Prism/Core/Ref.h"

#include <string>
#include <utility>
#include <glm/glm.hpp>

namespace Prism
{
    class RenderPass;
    class Pipeline;
    class Mesh;
    class Material;
    class TextureCube;
    class Image2D;
    class SceneEnvironment;

    using RendererID = uint32_t;

    enum class RendererAPIType
    {
        None = 0,
        OpenGL = 1
    };

    struct RenderAPICapabilities
    {
        std::string Vendor;
        std::string Renderer;
        std::string Version;

        int MaxSamples = 0;
        float MaxAnisotropy = 0.0f;
        int MaxTextureUnits = 0;

        int MaxGroupCount[3]{}, MaxGroupSize[3]{}, MaxInvocations{};
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

    public:
        virtual ~RendererAPI() = default;

        // 高层纯虚接口（对齐 Hazel，后端多态分发）
        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true) = 0;
        virtual void EndRenderPass() = 0;
        virtual void SubmitFullscreenQuad(Ref<Pipeline> pipeline, Ref<Material> material) = 0;

        virtual void SetSceneEnvironment(const Ref<SceneEnvironment>& environment, const Ref<Image2D>& shadow) = 0;
        virtual std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath) = 0;

        // RenderMesh 适配 Prism：从 DrawCommand 取 material + 单 submeshIndex + PSL pass（决策6，不引入 RenderMeshWithoutMaterial）
        virtual void RenderMesh(Ref<Pipeline> pipeline, Ref<Mesh> mesh, Ref<Material> material,
            uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass) = 0;
        virtual void RenderQuad(Ref<Pipeline> pipeline, Ref<Material> material, const glm::mat4& transform) = 0;

        virtual RenderAPICapabilities& GetCapabilities() = 0;

        // 低层静态（本阶段保留，Phase 6 收敛进 OpenGLRenderer Utils static 对齐 Hazel）
        static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        static void Clear(float r, float g, float b, float a);
        static void SetClearColor(float r, float g, float b, float a);
        static void DrawIndexed(uint32_t count, PrimitiveType type = PrimitiveType::Triangles, bool depthTest = true);
        static void DrawIndexedBaseVertex(uint32_t count, uint32_t baseIndex, uint32_t baseVertex, PrimitiveType type = PrimitiveType::Triangles);
        static void SetLineThickness(float thickness);
        static void MemoryBarriers(BarrierFlags flags);

        static RendererAPIType Current() { return s_CurrentRendererAPI; }
    private:
        static RendererAPIType s_CurrentRendererAPI;
    };
    using MBarrier = RendererAPI::Barrier;
}
