#include "prpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Shader/PrismShader.h"
#include "Shader/GlobalUniforms.h"

#include "SceneRenderer.h"
#include "RendererAPI.h"
#include "Renderer2D.h"
#include "Buffer/ObjectUniformBuffer.h"

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
        Ref<VertexBuffer> m_FullscreenQuadVertexBuffer;
        Ref<IndexBuffer> m_FullscreenQuadIndexBuffer;
        Ref<Pipeline> m_FullscreenQuadPipeline;
        ObjectUniformBuffer m_ObjectUBO;
    };

    static RendererData s_Data;

    void Renderer::Init()
    {
        s_Data.m_ShaderLibrary = Ref<ShaderLibrary>::Create();
        s_Data.m_ObjectUBO.Init();
        Renderer::Submit([]() { RendererAPI::Init(); });

        Renderer::GetShaderLibrary()->LoadAll("Assets/Shaders");

        GlobalUniforms::Init();
        SceneRenderer::Init();

        // Create fullscreen quad
        float x = -1;
        float y = -1;
        float width = 2, height = 2;
        struct QuadVertex
        {
            glm::vec3 Position;
            glm::vec2 TexCoord;
        };

        QuadVertex* data = new QuadVertex[4];

        data[0].Position = glm::vec3(x, y, 0.1f);
        data[0].TexCoord = glm::vec2(0, 0);

        data[1].Position = glm::vec3(x + width, y, 0.1f);
        data[1].TexCoord = glm::vec2(1, 0);

        data[2].Position = glm::vec3(x + width, y + height, 0.1f);
        data[2].TexCoord = glm::vec2(1, 1);

        data[3].Position = glm::vec3(x, y + height, 0.1f);
        data[3].TexCoord = glm::vec2(0, 1);

        {
            PipelineSpecification pipelineSpec;
            pipelineSpec.Layout = {
                { ShaderDataType::Float3, "a_Position" , VertexSemantic::Position},
                { ShaderDataType::Float2, "a_TexCoord" , VertexSemantic::TexCoord0}
            };
            s_Data.m_FullscreenQuadPipeline = Pipeline::Create(pipelineSpec);

            s_Data.m_FullscreenQuadVertexBuffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));

            uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
            s_Data.m_FullscreenQuadIndexBuffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
        }

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

    

    void Renderer::SubmitQuad(Ref<MaterialInstance> material, const glm::mat4& transform)
    {
        bool depthTest = true;
        if (material)
        {
            material->Bind();
            s_Data.m_ObjectUBO.SetModel(transform);
            s_Data.m_ObjectUBO.Upload();
            s_Data.m_ObjectUBO.Bind();
        }
        s_Data.m_FullscreenQuadVertexBuffer->Bind();
        s_Data.m_FullscreenQuadPipeline->Bind();
        s_Data.m_FullscreenQuadIndexBuffer->Bind();
        Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest);
    }
    void Renderer::SubmitFullscreenQuad(Ref<MaterialInstance> material)
    {
        bool depthTest = true;
        if (material)
            material->Bind();
        s_Data.m_FullscreenQuadVertexBuffer->Bind();
        s_Data.m_FullscreenQuadPipeline->Bind();
        s_Data.m_FullscreenQuadIndexBuffer->Bind();
        Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest);
    }
    void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
    {
        mesh->m_VertexBuffer->Bind();
        mesh->m_Pipeline->Bind();
        mesh->m_IndexBuffer->Bind();
        auto& materials = mesh->GetMaterials();
        for (Submesh& submesh : mesh->m_Submeshes)
        {
            auto material = overrideMaterial ? overrideMaterial : materials[submesh.MaterialIndex];
            auto shader = material->GetShader();

            s_Data.m_ObjectUBO.SetModel(transform * submesh.Transform);
            if (mesh->m_IsAnimated)
                s_Data.m_ObjectUBO.SetBones(mesh->m_BoneTransforms.data(), (uint32_t)mesh->m_BoneTransforms.size());
            s_Data.m_ObjectUBO.Upload();
            s_Data.m_ObjectUBO.Bind();

            uint32_t passCount = material->GetPassCount();
            for (uint32_t p = 0; p < passCount; p++)
            {
                if (p == 0)
                    material->Bind();
                else
                    material->BindPass(p);

                Renderer::Submit([submesh, material]() {
                    PR_PROFILE_SCOPE("DrawCall With Submesh");
                    glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
                });
            }
        }
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
