#include "prpch.h"
#include "Renderer.h"
#include "Shader/PrismShader.h"

#include "RendererAPI.h"
#include "Renderer2D.h"
#include "Material.h"

#include "Camera/Camera.h"
#include <glad/glad.h>
namespace Prism
{
    RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL; 
    struct RendererData
    {
        Ref<RenderPass> m_ActiveRenderPass;
        RenderCommandQueue m_CommandQueue;
        Ref<ShaderLibrary> m_ShaderLibrary;
    };

    static RendererData s_Data; 
    void Renderer::Init()
    {
        s_Data.m_ShaderLibrary = Ref<ShaderLibrary>::Create();
        Renderer::Submit([]() { RendererAPI::Init(); });

        Renderer::GetShaderLibrary()->LoadAll("Assets/Shaders"); 
        Renderer2D::Init();
    }

    Ref<ShaderLibrary> Renderer::GetShaderLibrary()
    {
        return s_Data.m_ShaderLibrary;
    }

    Renderer::Renderer()
    {
    }
    Renderer::~Renderer()
    {
    }

    void* Renderer::DataAllocate(const void* data, size_t size)
    {
        return GetRenderCommandQueue().DataAllocate(data, size);
    }

    void Renderer::Clear()
    {
        Renderer::Submit([]() {
            RendererAPI::Clear(0.0f, 0.0f, 0.0f, 1.0f);
        });
    }
    void Renderer::Clear(float r, float g, float b, float a /*= 1.0f*/)
    {
        Renderer::Submit([=]() {
            RendererAPI::Clear(r, g, b, a);
        });
    }
    void Renderer::SetClearColor(float r, float g, float b, float a)
    {
    }
    void Renderer::ClearMagenta()
    {
        Clear(1, 0, 1);
    } 
    void Renderer::DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest)
    {
        Renderer::Submit([=]() {
            RendererAPI::DrawIndexed(count, type, depthTest);
        });
    }
    void Renderer::DrawIndexedBaseVertex(uint32_t count, uint32_t baseIndex, uint32_t baseVertex, PrimitiveType type)
    {
        Renderer::Submit([=]() {
            RendererAPI::DrawIndexedBaseVertex(count, baseIndex, baseVertex, type);
        });
    }
    void Renderer::SetLineThickness(float thickness)
    {
        Renderer::Submit([=]() {
            RendererAPI::SetLineThickness(thickness);
        });
    }

    void Renderer::MemoryBarriers(RendererAPI::BarrierFlags flags)
    {
        Renderer::Submit([=]() {
            RendererAPI::MemoryBarriers(flags);
        });
    }

    void Renderer::WaitAndRender()
    {
        s_Data.m_CommandQueue.Execute();
    }

    void Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
    {
        PR_CORE_ASSERT(renderPass, "渲染通道不能为空！"); 
        s_Data.m_ActiveRenderPass = renderPass;
        renderPass->GetSpecification().TargetFramebuffer->Bind();
        if (clear)
        {
            const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
            Renderer::Submit([=]() {
                RendererAPI::Clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            });
        }
    }

    void Renderer::EndRenderPass()
    {
        PR_CORE_ASSERT(s_Data.m_ActiveRenderPass, "没有活动的渲染通道！您是否调用了两次 Renderer::EndRenderPass？");
        s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
        s_Data.m_ActiveRenderPass = nullptr;
    }

    void Renderer::DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color)
    {
        for (Submesh& submesh : mesh->m_Submeshes)
        {
            auto& aabb = submesh.BoundingBox;
            auto aabbTransform = transform * submesh.Transform;
            DrawAABB(aabb, aabbTransform);
        }
    }

    void Renderer::DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color /*= glm::vec4(1.0f)*/)
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

    RenderCommandQueue& Renderer::GetRenderCommandQueue()
    {
        return s_Data.m_CommandQueue;
    }

}
