#include "prpch.h"
#include "Renderer.h"

#include "RendererAPI.h"
#include "Material.h"
#include "RenderPass.h"
#include "Mesh.h"
#include "VertexInput.h"
#include "SceneEnvironment.h"
#include "Image.h"
#include "Texture.h"
#include "Buffer/Framebuffer.h"
#include "Platform/OpenGL/OpenGLRenderer.h"

#include "Camera/Camera.h"

namespace Prism
{
    RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL;

    static RendererAPI* s_RendererAPI = nullptr;

    static void InitRendererAPI()
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
            s_RendererAPI = new OpenGLRenderer();
            break;
        // case RendererAPIType::Vulkan:
        //     s_RendererAPI = new VulkanRenderer(); // TODO: Vulkan 后端
        //     break;
        default:
            PR_CORE_ASSERT(false, "未支持的 RendererAPI");
        }
    }

    struct RendererData
    {
        RenderCommandQueue m_CommandQueue;
    };

    static RendererData s_Data;

    void Renderer::Init()
    {
        InitRendererAPI();
        s_RendererAPI->Init();
    }

    void Renderer::Shutdown()
    {
        if (s_RendererAPI)
        {
            s_RendererAPI->Shutdown();
            delete s_RendererAPI;
            s_RendererAPI = nullptr;
        }
    }

    Renderer::Renderer() {}
    Renderer::~Renderer() {}

    void* Renderer::DataAllocate(const void* data, size_t size)
    {
        return GetRenderCommandQueue().DataAllocate(data, size);
    }

    void Renderer::Clear()
    {
        Renderer::Submit([]() { RendererAPI::Clear(0.0f, 0.0f, 0.0f, 1.0f); });
    }
    void Renderer::Clear(float r, float g, float b, float a)
    {
        Renderer::Submit([=]() { RendererAPI::Clear(r, g, b, a); });
    }
    void Renderer::SetClearColor(float r, float g, float b, float a) {}
    void Renderer::ClearMagenta() { Clear(1, 0, 1); }

    void Renderer::DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest)
    {
        Renderer::Submit([=]() { RendererAPI::DrawIndexed(count, type, depthTest); });
    }
    void Renderer::DrawIndexedBaseVertex(uint32_t count, uint32_t baseIndex, uint32_t baseVertex, PrimitiveType type)
    {
        Renderer::Submit([=]() { RendererAPI::DrawIndexedBaseVertex(count, baseIndex, baseVertex, type); });
    }
    void Renderer::SetLineThickness(float thickness)
    {
        Renderer::Submit([=]() { RendererAPI::SetLineThickness(thickness); });
    }
    void Renderer::MemoryBarriers(RendererAPI::BarrierFlags flags)
    {
        Renderer::Submit([=]() { RendererAPI::MemoryBarriers(flags); });
    }

    // 高层转发（多态分发到 s_RendererAPI）
    void Renderer::BeginFrame() { s_RendererAPI->BeginFrame(); }
    void Renderer::EndFrame() { s_RendererAPI->EndFrame(); }

    void Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear) { s_RendererAPI->BeginRenderPass(renderPass, clear); }
    void Renderer::EndRenderPass() { s_RendererAPI->EndRenderPass(); }
    void Renderer::SubmitFullscreenQuad(Ref<VertexInput> vertexInput, Ref<Material> material) { s_RendererAPI->SubmitFullscreenQuad(vertexInput, material); }
    void Renderer::SetSceneEnvironment(const Ref<SceneEnvironment>& environment, const Ref<Image2D>& shadow) { s_RendererAPI->SetSceneEnvironment(environment, shadow); }
    std::pair<Ref<TextureCube>, Ref<TextureCube>> Renderer::CreateEnvironmentMap(const std::string& filepath) { return s_RendererAPI->CreateEnvironmentMap(filepath); }
    void Renderer::RenderMesh(Ref<VertexInput> vertexInput, Ref<Mesh> mesh, Ref<Material> material, uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass) { s_RendererAPI->RenderMesh(vertexInput, mesh, material, submeshIndex, transform, pass); }
    void Renderer::RenderQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const glm::mat4& transform) { s_RendererAPI->RenderQuad(vertexInput, material, transform); }

    void Renderer::SetDefaultStencilState() { s_RendererAPI->SetDefaultStencilState(); }
    void Renderer::BeginOutlineWrite() { s_RendererAPI->BeginOutlineWrite(); }
    void Renderer::BeginOutlineDraw() { s_RendererAPI->BeginOutlineDraw(); }
    void Renderer::EndOutline() { s_RendererAPI->EndOutline(); }
    void Renderer::BeginColliderDebug() { s_RendererAPI->BeginColliderDebug(); }
    void Renderer::EndColliderDebug() { s_RendererAPI->EndColliderDebug(); }

    RenderAPICapabilities& Renderer::GetCapabilities() { return s_RendererAPI->GetCapabilities(); }

    void Renderer::WaitAndRender() { s_Data.m_CommandQueue.Execute(); }

#if 0
    void Renderer::DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color)
    {
        for (Submesh& submesh : mesh->m_Submeshes)
        {
            auto& aabb = submesh.BoundingBox;
            auto aabbTransform = transform * submesh.Transform;
            DrawAABB(aabb, aabbTransform);
        }
    }

    void Renderer::DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color)
    {
        glm::vec4 min = { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f };
        glm::vec4 max = { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f };

        glm::vec4 corners[8] =
        {
            transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f },
            transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f },
            transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f },
            transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f },

            transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f },
            transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f },
            transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f },
            transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f }
        };
        for (uint32_t i = 0; i < 4; i++)
            Renderer2D::DrawLine(corners[i], corners[(i + 1) % 4], color);

        for (uint32_t i = 0; i < 4; i++)
            Renderer2D::DrawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);

        for (uint32_t i = 0; i < 4; i++)
            Renderer2D::DrawLine(corners[i], corners[i + 4], color);
    }
#endif

    RenderCommandQueue& Renderer::GetRenderCommandQueue() { return s_Data.m_CommandQueue; }
}
