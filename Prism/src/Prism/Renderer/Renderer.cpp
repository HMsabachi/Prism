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

    void Renderer::BeginFrame() { s_RendererAPI->BeginFrame(); }
    void Renderer::EndFrame() { s_RendererAPI->EndFrame(); }

    void Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear) { s_RendererAPI->BeginRenderPass(renderPass, clear); }
    void Renderer::EndRenderPass() { s_RendererAPI->EndRenderPass(); }
    void Renderer::SubmitFullscreenQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const PrismShaderCompiler::PipelineState* stateOverride) { s_RendererAPI->SubmitFullscreenQuad(vertexInput, material, stateOverride); }
    void Renderer::SetSceneEnvironment(const Ref<SceneEnvironment>& environment) { s_RendererAPI->SetSceneEnvironment(environment); }
    void Renderer::SetUniformBuffer(uint32_t set, uint32_t binding, Ref<UniformBuffer> ubo) { s_RendererAPI->SetUniformBuffer(set, binding, ubo); }
    void Renderer::SetShaderStorageBuffer(uint32_t set, uint32_t binding, Ref<ShaderStorageBuffer> ssbo) { s_RendererAPI->SetShaderStorageBuffer(set, binding, ssbo); }
    void Renderer::SetTexture(uint32_t set, uint32_t binding, Ref<Image> image) { s_RendererAPI->SetTexture(set, binding, image); }
    std::pair<Ref<TextureCube>, Ref<TextureCube>> Renderer::CreateEnvironmentMap(const std::string& filepath) { return s_RendererAPI->CreateEnvironmentMap(filepath); }
    void Renderer::RenderMesh(Ref<Mesh> mesh, Ref<Material> material, uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass, const PrismShaderCompiler::PipelineState* stateOverride) { s_RendererAPI->RenderMesh(mesh, material, submeshIndex, transform, pass, stateOverride); }
    void Renderer::RenderQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const glm::mat4& transform, const PrismShaderCompiler::PipelineState* stateOverride) { s_RendererAPI->RenderQuad(vertexInput, material, transform, stateOverride); }

    void Renderer::DispatchCompute(Ref<Shader> kernelShader, const std::vector<ComputeResourceBinding>& bindings, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) { s_RendererAPI->DispatchCompute(kernelShader, bindings, numGroupsX, numGroupsY, numGroupsZ); }

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
