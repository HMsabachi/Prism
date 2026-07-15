#include "prpch.h"
#include "OpenGLRenderer.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/RenderPass.h"
#include "Prism/Renderer/VertexInput.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/SceneEnvironment.h"
#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "Prism/Renderer/Buffer/Framebuffer.h"
#include "Prism/Renderer/Buffer/IndexBuffer.h"
#include "OpenGLStateCache.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Prism
{
    struct OpenGLRendererData
    {
        RenderAPICapabilities RenderCaps;
        Ref<RenderPass> ActiveRenderPass;
        Ref<Shader> LastProgram;
        Ref<Material> LastMaterial;
        Ref<Mesh> LastMesh;
    };

    static OpenGLRendererData* s_Data = nullptr;
    static Ref<ComputeShader> s_EnvironmentShader;

    static void OpenGLLogMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
    {
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            PR_CORE_ERROR("[OpenGL Debug HIGH] {0}", message);
            PR_CORE_ASSERT(false, "GL_DEBUG_SEVERITY_HIGH");
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            PR_CORE_WARN("[OpenGL Debug MEDIUM] {0}", message);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            PR_CORE_INFO("[OpenGL Debug LOW] {0}", message);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            break;
        }
    }

    static void HandleCapabilities(RenderAPICapabilities& caps)
    {
        caps.Vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        caps.Renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        caps.Version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

        glGetIntegerv(GL_MAX_SAMPLES, &caps.MaxSamples);
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &caps.MaxAnisotropy);
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &caps.MaxTextureUnits);

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &caps.MaxGroupCount[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &caps.MaxGroupSize[0]);
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &caps.MaxInvocations);
    }

    void OpenGLRenderer::Init()
    {
        s_Data = new OpenGLRendererData();
        Renderer::Submit([]() {
            glDebugMessageCallback(OpenGLLogMessage, nullptr);
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

            // 临时创建 VAO
            unsigned int vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
            glFrontFace(GL_CCW);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_STENCIL_TEST);

            HandleCapabilities(s_Data->RenderCaps);

            GLenum error = glGetError();
            while (error != GL_NO_ERROR)
            {
                PR_CORE_ERROR("OpenGL Error {0}", error);
                error = glGetError();
            }

            OpenGLStateCache::Init();
        });
    }

    void OpenGLRenderer::Shutdown()
    {
        delete s_Data;
        s_Data = nullptr;
    }

    RenderAPICapabilities& OpenGLRenderer::GetCapabilities()
    {
        return s_Data->RenderCaps;
    }

    void OpenGLRenderer::BeginFrame() {}
    void OpenGLRenderer::EndFrame() {}

    void OpenGLRenderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
    {
        PR_CORE_ASSERT(renderPass, "渲染通道不能为空！");
        s_Data->ActiveRenderPass = renderPass;
        s_Data->LastProgram = nullptr;
        s_Data->LastMaterial = nullptr;
        s_Data->LastMesh = nullptr;
        renderPass->GetSpecification().TargetFramebuffer->Bind();
        if (clear)
        {
            const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
            Renderer::Submit([=]() {
                RendererAPI::Clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            });
        }
    }

    void OpenGLRenderer::EndRenderPass()
    {
        PR_CORE_ASSERT(s_Data->ActiveRenderPass, "没有活动的渲染通道！您是否调用了两次 Renderer::EndRenderPass？");
        s_Data->ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
        s_Data->ActiveRenderPass = nullptr;
    }

    void OpenGLRenderer::SubmitFullscreenQuad(Ref<VertexInput> vertexInput, Ref<Material> material)
    {
        // TODO: Phase 6 接入（fullscreen quad 当前在 RenderPipeline 持有）
    }

    void OpenGLRenderer::SetSceneEnvironment(const Ref<SceneEnvironment>& environment, const Ref<Image2D>& shadow)
    {
        // TODO: Phase 6 接入
    }

    std::pair<Ref<TextureCube>, Ref<TextureCube>>
    OpenGLRenderer::CreateEnvironmentMap(const std::string& filepath)
    {
        // 从 RenderPipeline.cpp 搬来，使用 Prism ComputeShader 高层 API（不照搬 Hazel 原始 GL）
        PR_PROFILE_FUNCTION();
        const uint32_t cubemapSize = 2048;
        const uint32_t irradianceMapSize = 32;

        Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
        if (!s_EnvironmentShader)
            s_EnvironmentShader = ComputeShader::Create("Assets/Shaders/Environment.ComputeShader");
        Ref<Texture2D> envEquirect = Texture2D::Create(filepath);
        PR_CORE_ASSERT(envEquirect->GetFormat() == TextureFormat::Float16, "Texture is not HDR!");

        envEquirect->Bind();
        int toCubeKernel = s_EnvironmentShader->FindKernel("CSEquirectToCube");
        s_EnvironmentShader->SetTexture2D(toCubeKernel, "u_EquirectangularTex", envEquirect);
        s_EnvironmentShader->SetImageCube(toCubeKernel, "o_OutputCube", envUnfiltered);
        s_EnvironmentShader->Dispatch(toCubeKernel, cubemapSize / 32, cubemapSize / 32, 6);
        envUnfiltered->GenerateMipMap();

        Ref<TextureCube> envFiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
        envUnfiltered->CopyTo(envFiltered);

        int mipFilter = s_EnvironmentShader->FindKernel("CSMipFilter");
        s_EnvironmentShader->SetTextureCube(mipFilter, "u_InputCubeMap", envUnfiltered);
        const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
        for (uint32_t level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2)
        {
            const uint32_t numGroups = glm::max((uint32_t)1, size / 32);
            s_EnvironmentShader->SetImageCube(mipFilter, "o_OutputCube", envFiltered, level, true);
            s_EnvironmentShader->SetFloat(mipFilter, "u_Roughness", level * deltaRoughness);
            s_EnvironmentShader->Dispatch(mipFilter, numGroups, numGroups, 6);
        }

        Ref<TextureCube> irradianceMap = TextureCube::Create(TextureFormat::Float16, irradianceMapSize, irradianceMapSize);
        int irradiance = s_EnvironmentShader->FindKernel("CSIrradiance");
        s_EnvironmentShader->SetTextureCube(irradiance, "u_InputCubeMap", envFiltered);
        s_EnvironmentShader->SetImageCube(irradiance, "o_OutputCube", irradianceMap);
        s_EnvironmentShader->Dispatch(irradiance, irradianceMapSize / 32, irradianceMapSize / 32, 6);
        irradianceMap->GenerateMipMap();

        return { envFiltered, irradianceMap };
    }

    void OpenGLRenderer::RenderMesh(Ref<VertexInput> vertexInput, Ref<Mesh> mesh, Ref<Material> material,
        uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass)
    {
        Ref<Shader> program = material->GetProgram(pass);
        if (program != s_Data->LastProgram)
        {
            material->BindProgram(pass);
            s_Data->LastProgram = program;
        }
        if (material != s_Data->LastMaterial)
        {
            material->BindUniform();
            material->BindTexture();
            s_Data->LastMaterial = material;
        }
        if (mesh != s_Data->LastMesh)
        {
            mesh->m_VertexBuffer->Bind();
            vertexInput->Bind();
            mesh->m_IndexBuffer->Bind();
            s_Data->LastMesh = mesh;
        }

        auto& submesh = mesh->m_Submeshes[submeshIndex];
        Renderer::Submit([submesh]() {
            glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT,
                (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
        });
    }

    void OpenGLRenderer::RenderQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const glm::mat4& transform)
    {
        // TODO: Phase 6 接入（fullscreen quad 当前在 RenderPipeline 持有）
    }

    void OpenGLRenderer::SetDefaultStencilState()
    {
        Renderer::Submit([]() {
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xff);
            glStencilMask(0);
        });
    }

    void OpenGLRenderer::BeginOutlineWrite()
    {
        s_Data->LastProgram = nullptr;
        s_Data->LastMaterial = nullptr;
        s_Data->LastMesh = nullptr;

        Renderer::Submit([]() {
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 0xff);
            glStencilMask(0xff);
        });
    }

    void OpenGLRenderer::BeginOutlineDraw()
    {
        s_Data->LastProgram = nullptr;
        s_Data->LastMaterial = nullptr;
        s_Data->LastMesh = nullptr;

        Renderer::Submit([]() {
            glStencilFunc(GL_NOTEQUAL, 1, 0xff);
            glStencilMask(0);
            glLineWidth(10);
            glEnable(GL_LINE_SMOOTH);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        });
    }

    void OpenGLRenderer::EndOutline()
    {
        Renderer::Submit([]() {
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xff);
            glStencilMask(0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        });
    }

    void OpenGLRenderer::BeginColliderDebug()
    {
        s_Data->LastProgram = nullptr;
        s_Data->LastMaterial = nullptr;
        s_Data->LastMesh = nullptr;

        Renderer::Submit([]() {
            glLineWidth(3);
            glEnable(GL_LINE_SMOOTH);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
        });
    }

    void OpenGLRenderer::EndColliderDebug()
    {
        Renderer::Submit([]() {
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xff);
            glStencilMask(0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
        });
    }
}
