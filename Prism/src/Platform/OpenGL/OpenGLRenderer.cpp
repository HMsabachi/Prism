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
#include "Prism/Renderer/Buffer/VertexBuffer.h"
#include "Prism/Renderer/Buffer/IndexBuffer.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Prism/ShaderCompiler/PrismBindings.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"
#include "OpenGLImage.h"
#include "OpenGLPipelineStateCache.h"
#include "Buffer/OpenGLUniformBuffer.h"
#include "Buffer/OpenGLShaderStorageBuffer.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Prism
{
    struct MaterialUBOEntry
    {
        Ref<UniformBuffer> UBO;
        uint32_t Size = 0;
    };

    struct OpenGLRendererData
    {
        RenderAPICapabilities RenderCaps;
        Ref<RenderPass> ActiveRenderPass;
        PSOKey LastPSOKey;
        Ref<Material> LastMaterial;
        Ref<Mesh> LastMesh;
        Ref<VertexBuffer> FullscreenQuadVB;
        Ref<IndexBuffer> FullscreenQuadIB;
        std::unordered_map<Material*, MaterialUBOEntry> MaterialUBOs;
        RendererID LastComputeProgram = 0;
        std::array<Ref<Image>, 9> GlobalTextures;
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

        static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            glViewport(x, y, width, height);
        }

        static void Clear(float r, float g, float b, float a)
        {
            glClearColor(r, g, b, a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        static void DrawIndexed(uint32_t count, PrimitiveType type)
        {
            glDrawElements(PrismToOpenGLPrimitiveType(type), count, GL_UNSIGNED_INT, nullptr);
        }

        static void DrawIndexedBaseVertex(uint32_t count, uint32_t baseIndex, uint32_t baseVertex, PrimitiveType type)
        {
            glDrawElementsBaseVertex(PrismToOpenGLPrimitiveType(type), count, GL_UNSIGNED_INT,
                (void*)(sizeof(uint32_t) * baseIndex), baseVertex);
        }

        static void SetLineThickness(float thickness)
        {
            glLineWidth(thickness);
        }

        static void MemoryBarriers(RendererAPI::BarrierFlags flags)
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
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_STENCIL_TEST);

            HandleCapabilities(s_Data->RenderCaps);

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
        uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
        s_Data->FullscreenQuadIB = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
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
        s_Data->LastPSOKey = PSOKey{};
        s_Data->LastMaterial = nullptr;
        s_Data->LastMesh = nullptr;
        renderPass->GetSpecification().TargetFramebuffer->Bind();
        if (clear)
        {
            const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
            Renderer::Submit([=]() {
                Utils::Clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
                });
        }
    }

    void OpenGLRenderer::EndRenderPass()
    {
        PR_CORE_ASSERT(s_Data->ActiveRenderPass, "没有活动的渲染通道！您是否调用了两次 Renderer::EndRenderPass？");
        s_Data->ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
        s_Data->ActiveRenderPass = nullptr;
    }

    void OpenGLRenderer::SubmitFullscreenQuad(Ref<VertexInput> vertexInput, Ref<Material> material,
        const PrismShaderCompiler::PipelineState* stateOverride)
    {
        BindMaterial(material, 0, stateOverride);
        s_Data->FullscreenQuadVB->Bind();
        vertexInput->Bind();
        s_Data->FullscreenQuadIB->Bind();
        s_Data->LastPSOKey = PSOKey{};
        s_Data->LastMaterial = nullptr;
        s_Data->LastMesh = nullptr;
        Renderer::Submit([]() {
            Utils::DrawIndexed(6, PrimitiveType::Triangles);
            });
    }

    void OpenGLRenderer::SetSceneEnvironment(const Ref<SceneEnvironment>& environment)
    {
        // TODO: Phase 6 接入
    }

    void OpenGLRenderer::SetGlobalTexture(uint32_t slot, Ref<Image> image)
    {
        s_Data->GlobalTextures[slot] = image;

        uint32_t binding = slot + Config::PRISM_OPENGL_GLOBAL_TEXTURE_BEGIN;
        if (auto* img2d = dynamic_cast<OpenGLImage2D*>(image.Raw()))
            img2d->Bind(binding);
        else if (auto* cube = dynamic_cast<OpenGLImageCube*>(image.Raw()))
            cube->Bind(binding);
    }

    std::pair<Ref<TextureCube>, Ref<TextureCube>> OpenGLRenderer::CreateEnvironmentMap(const std::string& filepath)
    {
        PR_PROFILE_FUNCTION();
        const uint32_t cubemapSize = 2048;
        const uint32_t irradianceMapSize = 32;

        Ref<TextureCube> envUnfiltered = TextureCube::Create(ImageFormat::RGBA16F, cubemapSize, cubemapSize);
        if (!s_EnvironmentShader)
            s_EnvironmentShader = ComputeShader::Create("Assets/Shaders/Environment.ComputeShader");
        Ref<Texture2D> envEquirect = Texture2D::Create(filepath);
        PR_CORE_ASSERT(envEquirect->GetFormat() == ImageFormat::RGBA16F, "Texture is not HDR!");

        int toCubeKernel = s_EnvironmentShader->FindKernel("CSEquirectToCube");
        s_EnvironmentShader->SetTexture(toCubeKernel, "u_EquirectangularTex", envEquirect);
        s_EnvironmentShader->SetImage(toCubeKernel, "o_OutputCube", envUnfiltered);
        s_EnvironmentShader->Dispatch(toCubeKernel, cubemapSize / 32, cubemapSize / 32, 6);
        envUnfiltered.As<OpenGLTextureCube>()->GenerateMipMap();

        Ref<TextureCube> envFiltered = TextureCube::Create(ImageFormat::RGBA16F, cubemapSize, cubemapSize);
        envUnfiltered.As<OpenGLTextureCube>()->CopyTo(envFiltered);

        Ref<UniformBuffer> mipFilterUBO = UniformBuffer::Create(4, sizeof(float));
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

        Ref<TextureCube> irradianceMap = TextureCube::Create(ImageFormat::RGBA16F, irradianceMapSize, irradianceMapSize);
        int irradiance = s_EnvironmentShader->FindKernel("CSIrradiance");
        s_EnvironmentShader->SetTexture(irradiance, "u_InputCubeMap", envFiltered);
        s_EnvironmentShader->SetImage(irradiance, "o_OutputCube", irradianceMap);
        s_EnvironmentShader->Dispatch(irradiance, irradianceMapSize / 32, irradianceMapSize / 32, 6);
        irradianceMap.As<OpenGLTextureCube>()->GenerateMipMap();

        return { envFiltered, irradianceMap };
    }

    void OpenGLRenderer::RenderMesh(Ref<VertexInput> vertexInput, Ref<Mesh> mesh, Ref<Material> material,
        uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass,
        const PrismShaderCompiler::PipelineState* stateOverride)
    {
        BindMaterial(material, pass, stateOverride);
        if (mesh != s_Data->LastMesh)
        {
            mesh->m_VertexBuffer->Bind();
            vertexInput->Bind();
            mesh->m_IndexBuffer->Bind();
            s_Data->LastMesh = mesh;
        }

        auto& submesh = mesh->m_Submeshes[submeshIndex];
        Renderer::Submit([submesh]() {
            Utils::DrawIndexedBaseVertex(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex, PrimitiveType::Triangles);
            });
    }

    void OpenGLRenderer::RenderQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const glm::mat4& transform,
        const PrismShaderCompiler::PipelineState* stateOverride)
    {
        (void)transform;
        SubmitFullscreenQuad(vertexInput, material, stateOverride);
    }

    void OpenGLRenderer::BindMaterial(Ref<Material> material, uint32_t pass,
        const PrismShaderCompiler::PipelineState* stateOverride)
    {
        if (!material)
            return;

        Ref<Shader> program = material->GetProgram(pass);
        PrismShaderCompiler::PipelineState effectiveState = PrismShaderCompiler::PipelineState::Default();
        const auto& shPass = material->GetShader()->GetPass(pass);
        if (shPass.RenderState)
            effectiveState = *shPass.RenderState;
        if (stateOverride)
            effectiveState.Merge(*stateOverride);
        PSOKey key{ program.As<OpenGLShader>()->GetRendererID(), effectiveState };
        if (!(key == s_Data->LastPSOKey))
        {
            OpenGLPipelineStateCache::Get(program.As<OpenGLShader>()->GetRendererID(), effectiveState)->Bind();
            s_Data->LastPSOKey = key;
        }

        if (material != s_Data->LastMaterial)
        {
            uint32_t size = (uint32_t)material->GetPropertyBuffer().GetSize();
            auto& entry = s_Data->MaterialUBOs[material.Raw()];
            if (!entry.UBO || entry.Size != size)
            {
                entry.UBO = UniformBuffer::Create(Config::PRISM_OPENGL_BINDING_MATERIAL, size);
                entry.Size = size;
                material->SetDirty(true);
            }
            if (material->IsDirty())
            {
                entry.UBO->SetData(material->GetPropertyBuffer());
                material->SetDirty(false);
            }
            entry.UBO->Bind();

            const auto& textures = material->GetTextures();
            for (const auto& [index, tex] : textures)
            {
                if (!tex) continue;
                uint32_t slot = index + Config::PRISM_OPENGL_TEXTURE_BEGIN_BINDING;
                if (tex->GetType() == TextureType::Texture2D)
                    tex.As<OpenGLTexture2D>()->GetImage().As<OpenGLImage2D>()->Bind(slot);
                else
                    tex.As<OpenGLTextureCube>()->GetImage().As<OpenGLImageCube>()->Bind(slot);
            }
            s_Data->LastMaterial = material;
        }
    }

    void OpenGLRenderer::DispatchCompute(Ref<Shader> kernelShader,
        const std::vector<ComputeResourceBinding>& bindings,
        uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
        if (!kernelShader)
            return;

        Ref<Shader> capturedShader = kernelShader;
        std::vector<ComputeResourceBinding> captured = bindings;

        Renderer::Submit([=]() {
            RendererID program = capturedShader.As<OpenGLShader>()->GetRendererID();
            if (s_Data->LastComputeProgram != program)
            {
                glUseProgram(program);
                s_Data->LastComputeProgram = program;
            }

            for (const auto& res : captured)
            {
                if (res.UBO)
                {
                    GLuint id = res.UBO.As<OpenGLUniformBuffer>()->GetRendererID();
                    glBindBufferBase(GL_UNIFORM_BUFFER, res.Binding, id);
                }
                else if (res.SSBO)
                {
                    GLuint id = res.SSBO.As<OpenGLShaderStorageBuffer>()->GetRendererID();
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, res.Binding, id);
                }
                else if (res.Texture)
                {
                    RendererID texId = 0;
                    GLenum fmt = Utils::TextureFormatToGL(res.Texture->GetFormat());
                    if (res.Texture->GetType() == TextureType::Texture2D)
                        texId = res.Texture.As<OpenGLTexture2D>()->GetRendererID();
                    else
                        texId = res.Texture.As<OpenGLTextureCube>()->GetRendererID();

                    if (res.Kind == ComputeBindingKind::Image)
                    {
                        GLenum access = Utils::ComputeAccessToGL(res.ReadOnly, res.WriteOnly);
                        glBindImageTexture(res.Binding, texId, res.Level, res.Layered, 0, access, fmt);
                    }
                    else
                    {
                        glBindTextureUnit(res.Binding, texId);
                    }
                }
            }

            glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        });
    }
}
