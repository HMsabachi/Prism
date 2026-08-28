#include "prpch.h"
#include "OpenGLRenderer.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/RenderPass.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/SceneEnvironment.h"
#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "Prism/Renderer/Buffer/Framebuffer.h"
#include "Prism/Renderer/Buffer/VertexBuffer.h"
#include "Prism/Renderer/Buffer/IndexBuffer.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Prism/ShaderCompiler/PrismBindings.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"
#include "OpenGLMaterialBackend.h"
#include "OpenGLImage.h"
#include "OpenGLPipelineState.h"
#include "OpenGLUniformBuffer.h"
#include "OpenGLShaderStorageBuffer.h"
#include "OpenGLFramebuffer.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLIndexBuffer.h"
#include "OpenGLVertexArray.h"
#include "OpenGLRenderPass.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Prism
{

    struct OpenGLRendererData
    {
        RenderAPICapabilities RenderCaps;
        OpenGLVertexArrayCache VertexArrayCache;
        Ref<RenderPass> ActiveRenderPass;
        Ref<Shader> LastProgram;
        Ref<Material> LastMaterial;
        Ref<Mesh> LastMesh;
        Ref<VertexBuffer> FullscreenQuadVB;
        Ref<IndexBuffer> FullscreenQuadIB;
        RendererID LastComputeProgram = 0;
        uint32_t TextureUnitTier = 32;
    };

    static OpenGLRendererData* s_Data = nullptr;
    static Ref<ComputeShader> s_EnvironmentShader;

    namespace Utils
    {
        static GLenum PrismToOpenGLPrimitiveType(PrimitiveType type)
        {
            switch (type)
            {
            case PrimitiveType::None:           PR_CORE_ASSERT(false, "Invalid PrimitiveType") return 0;
            case PrimitiveType::Triangles:      return GL_TRIANGLES;
            case PrimitiveType::Lines:          return GL_LINES;
            }
            PR_CORE_ASSERT(false, "Invalid PrimitiveType");
            return 0;
        }

        static GLbitfield PrismToOpenGLMemoryBarrier(RendererAPI::BarrierFlags flags)
        {
            GLbitfield result = 0;
            if (flags & RendererAPI::Barrier::ShaderStorage) result |= GL_SHADER_STORAGE_BARRIER_BIT;
            if (flags & RendererAPI::Barrier::VertexAttribArray) result |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
            if (flags & RendererAPI::Barrier::ElementArray) result |= GL_ELEMENT_ARRAY_BARRIER_BIT;
            if (flags & RendererAPI::Barrier::ImageAccess) result |= GL_FRAMEBUFFER_BARRIER_BIT;
            if (flags & RendererAPI::Barrier::TextureFetch) result |= GL_TEXTURE_FETCH_BARRIER_BIT;
            if (flags & RendererAPI::Barrier::TextureUpdate) result |= GL_TEXTURE_UPDATE_BARRIER_BIT;
            if (flags & RendererAPI::Barrier::Framebuffer) result |= GL_FRAMEBUFFER_BARRIER_BIT;
            if (flags & RendererAPI::Barrier::Command) result |= GL_COMMAND_BARRIER_BIT;
            if (flags & RendererAPI::Barrier::All) result |= GL_ALL_BARRIER_BITS;
            return result;
        }

        static void RT_SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            glViewport(x, y, width, height);
        }

        static void RT_Clear(float r, float g, float b, float a)
        {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glStencilMask(0xFF);
            glClearColor(r, g, b, a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        static void RT_DrawIndexed(uint32_t count, PrimitiveType type)
        {
            glDrawElements(PrismToOpenGLPrimitiveType(type), count, GL_UNSIGNED_INT, nullptr);
        }

        static void RT_DrawIndexedBaseVertex(uint32_t count, uint32_t baseIndex, uint32_t baseVertex, PrimitiveType type)
        {
            glDrawElementsBaseVertex(PrismToOpenGLPrimitiveType(type), count, GL_UNSIGNED_INT,
                (void*)(sizeof(uint32_t) * baseIndex), baseVertex);
        }

        static void RT_MemoryBarriers(RendererAPI::BarrierFlags flags)
        {
            glMemoryBarrier(PrismToOpenGLMemoryBarrier(flags));
        }

        static GLenum TextureFormatToGL(ImageFormat format)
        {
            switch (format)
            {
            case ImageFormat::RGB:     return GL_RGB;
            case ImageFormat::SRGB:    return GL_SRGB8;
            case ImageFormat::RGBA:    return GL_RGBA;
            case ImageFormat::RGBA16F: return GL_RGBA16F;
            case ImageFormat::RGBA32F: return GL_RGBA32F;
            }
            PR_CORE_ASSERT(false, "Unknown texture format!");
            return 0;
        }

        static GLenum TextureAccessToGL(TextureAccess access)
        {
            switch (access)
            {
            case TextureAccess::ReadOnly:  return GL_READ_ONLY;
            case TextureAccess::WriteOnly: return GL_WRITE_ONLY;
            case TextureAccess::ReadWrite: return GL_READ_WRITE;
            }
            PR_CORE_ASSERT(false, "Unknown texture access!");
            return 0;
        }

        static GLenum ComputeAccessToGL(bool readOnly, bool writeOnly)
        {
            if (readOnly && !writeOnly)  return GL_READ_ONLY;
            if (writeOnly && !readOnly)  return GL_WRITE_ONLY;
            return GL_READ_WRITE;
        }
    }

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

    static void RT_HandleCapabilities(RenderAPICapabilities& caps)
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

        GLint maxFragmentTextureUnits = 0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxFragmentTextureUnits);
        s_Data->TextureUnitTier = (maxFragmentTextureUnits >= 32) ? 32 : 16;
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
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_STENCIL_TEST);

            RT_HandleCapabilities(s_Data->RenderCaps);

            GLenum error = glGetError();
            while (error != GL_NO_ERROR)
            {
                PR_CORE_ERROR("OpenGL Error {0}", error);
                error = glGetError();
            }
            });

        struct QuadVertex
        {
            glm::vec3 Position;
            glm::vec2 TexCoord;
        };
        float x = -1.0f, y = -1.0f, width = 2.0f, height = 2.0f;
        QuadVertex data[4] = {
            { { x,         y,          0.1f }, { 0.0f, 0.0f } },
            { { x + width, y,          0.1f }, { 1.0f, 0.0f } },
            { { x + width, y + height, 0.1f }, { 1.0f, 1.0f } },
            { { x,         y + height, 0.1f }, { 0.0f, 1.0f } },
        };
        s_Data->FullscreenQuadVB = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
        VertexBufferLayout layout = {
            { ShaderDataType::Float3, "a_Position",  VertexSemantic::Position },
            { ShaderDataType::Float2, "a_TexCoord",  VertexSemantic::TexCoord0 }
        };
        s_Data->FullscreenQuadVB->SetLayout(layout);
        uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
        s_Data->FullscreenQuadIB = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
    }

    void OpenGLRenderer::Shutdown()
    {
        s_EnvironmentShader = nullptr;
        Renderer::SubmitResourceFree([=]() {
            delete s_Data;
            s_Data = nullptr;
        });
    }

    RenderAPICapabilities& OpenGLRenderer::GetCapabilities()
    {
        return s_Data->RenderCaps;
    }

    void OpenGLRenderer::BeginFrame()
    {
        Renderer::Submit([]() {
            const uint32_t releaseIndex = Renderer::GetCurrentFrameIndex() % Renderer::GetConfig().FramesInFlight;
            Renderer::GetRenderResourceReleaseQueue(releaseIndex).Execute();
        });
    }
    void OpenGLRenderer::EndFrame() {}

    void OpenGLRenderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
    {
        Renderer::Submit([=]()
        {
            s_Data->ActiveRenderPass = renderPass;
            s_Data->LastMaterial = nullptr;
            s_Data->LastMesh = nullptr;
            s_Data->LastProgram = nullptr;
            renderPass->GetSpecification().TargetFramebuffer.As<OpenGLFramebuffer>()->RT_Bind();
            if (clear)
            {
                const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
                Utils::RT_Clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            }
            renderPass.As<OpenGLRenderPass>()->RT_BindInputs();
        });
    }

    void OpenGLRenderer::EndRenderPass()
    {
        Renderer::Submit([]()
        {
            PR_CORE_ASSERT(s_Data->ActiveRenderPass, "没有活动的渲染通道！您是否调用了两次 Renderer::EndRenderPass？");
            s_Data->ActiveRenderPass->GetSpecification().TargetFramebuffer.As<OpenGLFramebuffer>()->RT_Unbind();
            s_Data->ActiveRenderPass = nullptr;
        });
        
    }

    void OpenGLRenderer::SubmitFullscreenQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex)
    {
        Renderer::Submit([=]() {
            RT_BindMaterial(material, passIndex);
            glUniform1i(0, drawIndex);
            s_Data->VertexArrayCache.RT_Get(s_Data->FullscreenQuadVB)->RT_Bind();
            s_Data->FullscreenQuadIB.As<OpenGLIndexBuffer>()->RT_Bind();
            s_Data->LastMaterial = nullptr;
            s_Data->LastMesh = nullptr;
            s_Data->LastProgram = nullptr;
            Utils::RT_DrawIndexed(6, PrimitiveType::Triangles);
        });
    }

    void OpenGLRenderer::SetSceneEnvironment(const Ref<SceneEnvironment>& environment)
    {
        // TODO: Phase 6 接入
    }

    void OpenGLRenderer::SetGlobalUniformBuffer(uint32_t binding, Ref<UniformBuffer> ubo)
    {
        Renderer::Submit([binding, ubo]() {
            uint32_t point = Config::GL_UBO_BASE_FRAME + binding;
            GLuint id = ubo.As<OpenGLUniformBuffer>()->GetRendererID();
            glBindBufferBase(GL_UNIFORM_BUFFER, point, id);
        });
    }

    void OpenGLRenderer::SetGlobalShaderStorageBuffer(uint32_t binding, Ref<ShaderStorageBuffer> ssbo)
    {
        Renderer::Submit([binding, ssbo]() {
            uint32_t point = Config::GL_SSBO_BASE_FRAME + binding;
            GLuint id = ssbo.As<OpenGLShaderStorageBuffer>()->GetRendererID();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, point, id);
        });
    }

    void OpenGLRenderer::SetGlobalTexture(uint32_t binding, Ref<Image> image)
    {
        Renderer::Submit([binding, image]() {
            uint32_t unit = Config::GL_TEX_BASE_FRAME + binding;
            if (auto* img2d = dynamic_cast<OpenGLImage2D*>(const_cast<Image*>(image.Raw())))
                glBindTextureUnit(unit, img2d->GetRendererID());
            else if (auto* cube = dynamic_cast<OpenGLImageCube*>(const_cast<Image*>(image.Raw())))
                glBindTextureUnit(unit, cube->GetRendererID());
        });
    }


    void OpenGLRenderer::BakeGlobalInputs() { } // OpenGL 不需要显式的 Bake

    std::pair<Ref<TextureCube>, Ref<TextureCube>> OpenGLRenderer::CreateEnvironmentMap(const std::string& filepath)
    {
        PR_PROFILE_FUNCTION();
        const uint32_t cubemapSize = 2048;
        const uint32_t irradianceMapSize = 32;

        Ref<TextureCube> envUnfiltered = TextureCube::Create(ImageFormat::RGBA32F, cubemapSize, cubemapSize);
        if (!s_EnvironmentShader)
            s_EnvironmentShader = ComputeShader::Create("Assets/Shaders/Environment.ComputeShader");
        Ref<Texture2D> envEquirect = Texture2D::Create(filepath);
        PR_CORE_ASSERT(envEquirect->GetFormat() == ImageFormat::RGBA32F, "Texture is not HDR!");

        int toCubeKernel = s_EnvironmentShader->FindKernel("CSEquirectToCube");
        s_EnvironmentShader->SetTexture(toCubeKernel, "u_EquirectangularTex", envEquirect);
        s_EnvironmentShader->SetImage(toCubeKernel, "o_OutputCube", envUnfiltered);
        s_EnvironmentShader->Dispatch(toCubeKernel, cubemapSize / 32, cubemapSize / 32, 6);
        envUnfiltered.As<OpenGLTextureCube>()->GenerateMipMap();

        Ref<TextureCube> envFiltered = TextureCube::Create(ImageFormat::RGBA32F, cubemapSize, cubemapSize);
        envUnfiltered.As<OpenGLTextureCube>()->CopyTo(envFiltered);

        Ref<UniformBuffer> mipFilterUBO = UniformBuffer::Create(sizeof(float));
        int mipFilter = s_EnvironmentShader->FindKernel("CSMipFilter");
        s_EnvironmentShader->SetTexture(mipFilter, "u_InputCubeMap", envUnfiltered);
        const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
        for (uint32_t level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2)
        {
            const uint32_t numGroups = glm::max((uint32_t)1, size / 32);
            s_EnvironmentShader->SetImage(mipFilter, "o_OutputCube", envFiltered, level);
            float roughness = level * deltaRoughness;
            mipFilterUBO->SetData(&roughness, sizeof(float));
            s_EnvironmentShader->SetUniformBuffer(mipFilter, "MipFilterParams", mipFilterUBO);
            s_EnvironmentShader->Dispatch(mipFilter, numGroups, numGroups, 6);
        }

        Ref<TextureCube> irradianceMap = TextureCube::Create(ImageFormat::RGBA32F, irradianceMapSize, irradianceMapSize);
        int irradiance = s_EnvironmentShader->FindKernel("CSIrradiance");
        s_EnvironmentShader->SetTexture(irradiance, "u_InputCubeMap", envFiltered);
        s_EnvironmentShader->SetImage(irradiance, "o_OutputCube", irradianceMap);
        s_EnvironmentShader->Dispatch(irradiance, irradianceMapSize / 32, irradianceMapSize / 32, 6);
        irradianceMap.As<OpenGLTextureCube>()->GenerateMipMap();

        return { envFiltered, irradianceMap };
    }

    void OpenGLRenderer::RenderMesh(Ref<Mesh> mesh, uint32_t submeshIndex, Ref<Material> material, uint32_t passIndex, uint32_t drawIndex)
    {
        Renderer::Submit([=]() mutable {
            RT_BindMaterial(material, passIndex);
            glUniform1i(0, drawIndex);
            if (mesh != s_Data->LastMesh)
            {
                s_Data->VertexArrayCache.RT_Get(mesh->m_VertexBuffer)->RT_Bind();
                mesh->m_IndexBuffer.As<OpenGLIndexBuffer>()->RT_Bind();
                s_Data->LastMesh = mesh;
            }
            auto& submesh = mesh->m_Submeshes[submeshIndex];
            Utils::RT_DrawIndexedBaseVertex(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex, PrimitiveType::Triangles);
        });
    }

    void OpenGLRenderer::RenderQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex)
    {
        SubmitFullscreenQuad(material, passIndex, drawIndex);
    }


    void OpenGLRenderer::RT_BindMaterial(Ref<Material> material, uint32_t passIndex)
    {
        if (!material) return;
        if (material != s_Data->LastMaterial)
        {
            const Ref<Shader>& program = material->GetProgram(passIndex);
            const auto& renderState = material->GetRenderState(passIndex);
            OpenGLPipelineState::RT_SetupPipelineState(renderState);
            s_Data->LastMaterial = material;
            if (!(program == s_Data->LastProgram))
            {
                program.As<OpenGLShader>()->RT_Bind();
                s_Data->LastProgram = program;
            }
            for (const auto& [index, tex] : material->GetTextures())
            {
                if (!tex) continue;
                uint32_t unit = Config::GL_TEX_BASE_MATERIAL + index;
                if (tex->GetType() == TextureType::Texture2D)
                    tex.As<OpenGLTexture2D>()->GetImage().As<OpenGLImage2D>()->RT_Bind(unit);
                else
                    tex.As<OpenGLTextureCube>()->GetImage().As<OpenGLImageCube>()->RT_Bind(unit);
            }
            const Ref<UniformBuffer>& ubo = material->RT_GetBackend().As<OpenGLMaterialBackend>()->RT_GetUniformBuffer();
            glBindBufferBase(GL_UNIFORM_BUFFER, Config::GL_UBO_BASE_MATERIAL, ubo.As<OpenGLUniformBuffer>()->GetRendererID());
        }
    }

    void OpenGLRenderer::DispatchCompute(Ref<ComputeShader> computeShader, int32_t kernel,
        uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
        if (!computeShader)
            return;
        Ref<Shader> kernelShader = computeShader->GetKernelShader(kernel);
        if (!kernelShader)
            return;

        std::vector<ComputeResourceBinding> captured = computeShader->GetResources();

        Renderer::Submit([=]() {
            RendererID program = kernelShader.As<OpenGLShader>()->GetRendererID();
            if (s_Data->LastComputeProgram != program)
            {
                glUseProgram(program);
                s_Data->LastComputeProgram = program;
            }

            using K = PrismShaderCompiler::CSL::ResourceKind;
            for (const auto& res : captured)
            {
                if (!res.res)
                    continue;

                if (res.Resource.Kind == K::UniformBuffer)
                {
                    Ref<UniformBuffer> ubo = res.res.As<UniformBuffer>();
                    glBindBufferBase(GL_UNIFORM_BUFFER, res.Resource.Binding, ubo.As<OpenGLUniformBuffer>()->GetRendererID());
                }
                else if (res.Resource.Kind == K::StorageBuffer)
                {
                    Ref<ShaderStorageBuffer> ssbo = res.res.As<ShaderStorageBuffer>();
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, res.Resource.Binding, ssbo.As<OpenGLShaderStorageBuffer>()->GetRendererID());
                }
                else
                {
                    Ref<Texture> tex = res.res.As<Texture>();
                    if (!tex)
                        continue;

                    RendererID texId = 0;
                    GLenum fmt = Utils::TextureFormatToGL(tex->GetFormat());
                    if (tex->GetType() == TextureType::Texture2D)
                        texId = tex.As<OpenGLTexture2D>()->GetRendererID();
                    else
                        texId = tex.As<OpenGLTextureCube>()->GetRendererID();

                    if (res.Resource.Kind == K::Image2D || res.Resource.Kind == K::Image3D || res.Resource.Kind == K::ImageCube)
                    {
                        GLenum access = Utils::ComputeAccessToGL(res.Resource.ReadOnly, res.Resource.WriteOnly);
                        glBindImageTexture(res.Resource.Binding, texId, res.Level, res.Resource.Kind == K::ImageCube, 0, access, fmt);
                    }
                    else
                    {
                        glBindTextureUnit(res.Resource.Binding, texId);
                    }
                }
            }

            glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        });
    }
}
