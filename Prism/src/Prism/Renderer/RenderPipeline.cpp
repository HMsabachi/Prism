#include "prpch.h"
#include "RenderPipeline.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Shader/PrismShader.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "Prism/ShaderCompiler/PrismBindings.h"
#include "Prism/Renderer/Camera/Camera.h"

#include <glad/glad.h>

namespace Prism
{
    static Ref<ComputeShader> s_EnvironmentShader;

    void RenderPipeline::Initialize(uint32_t viewportWidth, uint32_t viewportHeight)
    {
        m_FrameUBO.Init();
        m_ObjectUBO.Init();

        FramebufferSpecification geoFBSpec;
        geoFBSpec.Width = viewportWidth;
        geoFBSpec.Height = viewportHeight;
        geoFBSpec.Format = FramebufferFormat::RGBA16F;
        geoFBSpec.Samples = 8;
        geoFBSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification geoRPSpec;
        geoRPSpec.TargetFramebuffer = Framebuffer::Create(geoFBSpec);
        m_GeoPass = RenderPass::Create(geoRPSpec);

        FramebufferSpecification compFBSpec;
        compFBSpec.Width = viewportWidth;
        compFBSpec.Height = viewportHeight;
        compFBSpec.Format = FramebufferFormat::RGBA8;
        compFBSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification compRPSpec;
        compRPSpec.TargetFramebuffer = Framebuffer::Create(compFBSpec);
        m_CompositePass = RenderPass::Create(compRPSpec);

        m_CompositeShader = Renderer::GetShaderLibrary()->Get("Custom/SceneComposite");
        m_CompositeMaterial = Material::Create(m_CompositeShader);
        m_BRDFLUT = Texture2D::Create("Assets/Textures/BRDF_LUT.tga");

        auto gridShader = Renderer::GetShaderLibrary()->Get("Custom/Grid");
        m_GridMaterial = Material::Create(gridShader);
        m_GridMaterial->SetFloat("u_Scale", 16.025f);
        m_GridMaterial->SetFloat("u_Res", 0.025f);

        auto outlineShader = Renderer::GetShaderLibrary()->Get("Standard/Outline");
        m_OutlineMaterial = Material::Create(outlineShader);

        auto colliderShader = Renderer::GetShaderLibrary()->Get("Debug/Collider");
        m_ColliderMaterial = Material::Create(colliderShader);

        FramebufferSpecification shadowFBSpec;
        shadowFBSpec.Width = SHADOW_MAP_SIZE;
        shadowFBSpec.Height = SHADOW_MAP_SIZE;
        shadowFBSpec.Format = FramebufferFormat::Depth;
        for (int i = 0; i < 4; i++)
        {
            RenderPassSpecification shadowRPSpec;
            shadowRPSpec.TargetFramebuffer = Framebuffer::Create(shadowFBSpec);
            m_ShadowPasses[i] = RenderPass::Create(shadowRPSpec);
        }

        auto shadowShader = Renderer::GetShaderLibrary()->Get("Hidden/ShadowDepth");
        m_ShadowDepthMaterial = Material::Create(shadowShader);
    }

    void RenderPipeline::Shutdown()
    {
        m_GeoPass.Reset();
        m_CompositePass.Reset();
        for (int i = 0; i < 4; i++)
            m_ShadowPasses[i].Reset();
    }

    void RenderPipeline::Resize(uint32_t width, uint32_t height)
    {
        m_GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
        m_CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
    }

    void RenderPipeline::Execute(const RenderConfig& config, FrameData& data)
    {
        std::sort(data.DrawList.begin(), data.DrawList.end(),
            [](auto& a, auto& b) { return a.SortKey < b.SortKey; });

        if (config.ShadowsEnabled && !data.DrawList.empty())
            UpdateShadowData(config, data);
        BeginFrame(config, data);
        if (config.ShadowsEnabled && !data.DrawList.empty())
            ShadowPass(data.DrawList);
        GeometryPass(config, data.DrawList, data.SelectedDrawList, data.DebugDrawList);
        CompositePass();
    }

    void RenderPipeline::BeginFrame(const RenderConfig& config, const FrameData& data)
    {
        auto& cam = data.Camera;
        m_FrameUBO.SetViewProjection(cam.Projection.GetProjectionMatrix() * cam.ViewMatrix);
        m_FrameUBO.SetView(cam.ViewMatrix);
        m_FrameUBO.SetProjection(cam.Projection.GetProjectionMatrix());
        m_FrameUBO.SetCameraPosition(glm::inverse(cam.ViewMatrix)[3]);
        m_FrameUBO.SetTime(Time::GetTime(), Time::GetDeltaTime());
        m_FrameUBO.SetLight(0, config.SceneLight.Direction,
                            config.SceneLight.Radiance, config.SceneLight.Multiplier);
        m_FrameUBO.Upload();
        m_FrameUBO.Bind();

        config.SceneEnvironment.RadianceMap->Bind(Config::PRISM_ENV_RADIANCE);
        config.SceneEnvironment.IrradianceMap->Bind(Config::PRISM_ENV_IRRADIANCE);
        m_BRDFLUT->Bind(Config::PRISM_ENV_BRDF_LUT);
    }

    void RenderPipeline::UpdateShadowData(const RenderConfig& config, const FrameData& data)
    {
        auto& camera = data.Camera;
        auto& light = config.SceneLight;
        uint32_t cascadeCount = glm::clamp(config.CascadeCount, 1u, 4u);

        auto& proj = camera.Projection.GetProjectionMatrix();
        float f = proj[1][1];
        float fov = 2.0f * atan(1.0f / f);
        float aspect = proj[1][1] / proj[0][0];
        float nearClip = proj[3][2] / (proj[2][2] - 1.0f);
        float farClip  = proj[3][2] / (proj[2][2] + 1.0f);
        farClip = glm::min(farClip, config.MaxShadowDistance);

        float splits[4] = {};
        float splitLambda = 0.95f;
        for (uint32_t i = 0; i < cascadeCount; i++)
        {
            float fraction = (float)(i + 1) / (float)cascadeCount;
            float logSplit = nearClip * pow(farClip / nearClip, fraction);
            float uniSplit = nearClip + (farClip - nearClip) * fraction;
            splits[i] = logSplit * splitLambda + uniSplit * (1.0f - splitLambda);
        }

        m_CascadeSplits = glm::vec4(splits[0], splits[1], splits[2], splits[3]);
        glm::vec4 shadowParams(config.ShadowBias, config.ShadowNormalBias,
                               (float)cascadeCount, 0.0f);

        glm::mat4 invView = glm::inverse(camera.ViewMatrix);
        glm::vec3 lightDir = glm::normalize(light.Direction);
        glm::vec3 up = glm::abs(lightDir.y) < 0.99f
                           ? glm::vec3(0.0f, 1.0f, 0.0f)
                           : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::mat4 lightView = glm::lookAt(-lightDir, glm::vec3(0.0f), up);

        float tanHalfFov = tanf(fov * 0.5f);
        float prevSplit = nearClip;

        for (uint32_t cascade = 0; cascade < cascadeCount; cascade++)
        {
            float nextSplit = splits[cascade];
            float k = sqrtf(1.0f + aspect * aspect) * tanHalfFov;
            float centerZ, radius;

            if (k * k >= (nextSplit - prevSplit) / (nextSplit + prevSplit))
            {
                centerZ = nextSplit;
                radius = nextSplit * k;
            }
            else
            {
                centerZ = 0.5f * (prevSplit + nextSplit) * (1.0f + k * k);
                radius = 0.5f * sqrtf(
                    powf(nextSplit - prevSplit, 2.0f) +
                    2.0f * (nextSplit * nextSplit + prevSplit * prevSplit) * k * k +
                    powf(nextSplit + prevSplit, 2.0f) * powf(k, 4.0f));
            }

            glm::vec3 sphereCenterWorld = glm::vec3(invView * glm::vec4(0.0f, 0.0f, -centerZ, 1.0f));
            glm::vec3 sphereCenterLight = glm::vec3(lightView * glm::vec4(sphereCenterWorld, 1.0f));

            float minX = sphereCenterLight.x - radius;
            float maxX = sphereCenterLight.x + radius;
            float minY = sphereCenterLight.y - radius;
            float maxY = sphereCenterLight.y + radius;
            float minZ = sphereCenterLight.z - radius - 200.0f;
            float maxZ = sphereCenterLight.z + radius + 200.0f;

            float worldTexelSize = (2.0f * radius) / (float)SHADOW_MAP_SIZE;
            minX = floorf(minX / worldTexelSize) * worldTexelSize;
            minY = floorf(minY / worldTexelSize) * worldTexelSize;
            maxX = minX + (2.0f * radius);
            maxY = minY + (2.0f * radius);

            m_ShadowMatrices[cascade] = glm::ortho(minX, maxX, minY, maxY, -maxZ, -minZ) * lightView;
            prevSplit = nextSplit;
        }

        m_FrameUBO.SetShadowMatrices(m_ShadowMatrices, cascadeCount);
        m_FrameUBO.SetCascadeSplits(m_CascadeSplits);
        m_FrameUBO.SetShadowParams(shadowParams);
    }

    void RenderPipeline::ShadowPass(const std::vector<DrawCommand>& drawList)
    {
        PR_PROFILE_FUNCTION();
        if (drawList.empty()) return;

        for (uint32_t cascade = 0; cascade < 4; cascade++)
        {
            Renderer::BeginRenderPass(m_ShadowPasses[cascade]);

            m_ShadowDepthMaterial->SetMatrix4("u_LightVP", m_ShadowMatrices[cascade]);
            m_ShadowDepthMaterial->Bind();

            Ref<Mesh> boundMesh = nullptr;
            for (auto& dc : drawList)
            {
                if (dc.Mesh != boundMesh)
                {
                    dc.Mesh->m_VertexBuffer->Bind();
                    dc.Mesh->m_Pipeline->Bind();
                    dc.Mesh->m_IndexBuffer->Bind();
                    boundMesh = dc.Mesh;
                }

                m_ObjectUBO.SetModel(dc.Transform);
                m_ObjectUBO.Upload();
                m_ObjectUBO.Bind();

                auto& submesh = dc.Mesh->m_Submeshes[dc.SubmeshIndex];
                Renderer::DrawIndexedBaseVertex(
                    submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            }

            Renderer::EndRenderPass();
        }
    }

    void RenderPipeline::GeometryPass(
        const RenderConfig config,
        const std::vector<DrawCommand>& drawList,
        const std::vector<DrawCommand>& selectedList,
        const std::vector<DrawCommand>& debugList)
    {
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(m_GeoPass);

        Renderer::Submit([]() {
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilMask(0);
        });

        Renderer::SubmitFullscreenQuad(config.SkyboxMaterial);

        for (int i = 0; i < 4; i++)
            m_ShadowPasses[i]->GetSpecification().TargetFramebuffer->BindDepthTexture(Config::PRISM_SHADOW_MAP0 + i);

        if (!drawList.empty())
        {
            Ref<Shader> boundProgram;
            Ref<Material> boundMaterial;
            Ref<Mesh> boundMesh;

            for (auto& dc : drawList)
            {
                auto material = dc.Material ? dc.Material : Renderer::GetDefaultMaterial();
                if (boundProgram != material->GetProgram())
                {
                    material->BindProgram();
                    boundProgram = material->GetProgram();
                }
                if (material != boundMaterial)
                {
                    material->BindUniform();
                    material->BindTexture();
                    boundMaterial = material;
                }

                if (dc.Mesh != boundMesh)
                {
                    dc.Mesh->m_VertexBuffer->Bind();
                    dc.Mesh->m_Pipeline->Bind();
                    dc.Mesh->m_IndexBuffer->Bind();
                    boundMesh = dc.Mesh;
                }

                m_ObjectUBO.SetModel(dc.Transform);
                m_ObjectUBO.Upload();
                m_ObjectUBO.Bind();

                auto& submesh = dc.Mesh->m_Submeshes[dc.SubmeshIndex];
                Renderer::DrawIndexedBaseVertex(
                    submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            }
        }

        // Selected outline
        if (!selectedList.empty())
        {
            Renderer::Submit([]() {
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, 1, 0xff);
                glStencilMask(0xff);
            });

            {
                Ref<Material> boundMaterial;
                Ref<Mesh> boundMesh;

                for (auto& dc : selectedList)
                {
                    auto material = dc.Material ? dc.Material : Renderer::GetDefaultMaterial();

                    if (material != boundMaterial)
                    {
                        material->Bind();
                        boundMaterial = material;
                    }
                    if (dc.Mesh != boundMesh)
                    {
                        dc.Mesh->m_VertexBuffer->Bind();
                        dc.Mesh->m_Pipeline->Bind();
                        dc.Mesh->m_IndexBuffer->Bind();
                        boundMesh = dc.Mesh;
                    }

                    m_ObjectUBO.SetModel(dc.Transform);
                    m_ObjectUBO.Upload();
                    m_ObjectUBO.Bind();

                    auto& submesh = dc.Mesh->m_Submeshes[dc.SubmeshIndex];
                    Renderer::DrawIndexedBaseVertex(
                        submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                }
            }

            Renderer::Submit([]() {
                glStencilFunc(GL_NOTEQUAL, 1, 0xff);
                glStencilMask(0);
                glLineWidth(10);
                glEnable(GL_LINE_SMOOTH);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            });

            {
                Ref<Mesh> boundMesh = nullptr;
                m_OutlineMaterial->Bind();

                for (auto& dc : selectedList)
                {
                    if (dc.Mesh != boundMesh)
                    {
                        dc.Mesh->m_VertexBuffer->Bind();
                        dc.Mesh->m_Pipeline->Bind();
                        dc.Mesh->m_IndexBuffer->Bind();
                        boundMesh = dc.Mesh;
                    }

                    m_ObjectUBO.SetModel(dc.Transform);
                    m_ObjectUBO.Upload();
                    m_ObjectUBO.Bind();

                    auto& submesh = dc.Mesh->m_Submeshes[dc.SubmeshIndex];
                    Renderer::DrawIndexedBaseVertex(
                        submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                }
            }

            Renderer::Submit([]() {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glStencilMask(0xff);
                glStencilFunc(GL_ALWAYS, 1, 0xff);
            });
        }

        // Collider debug
        if (!debugList.empty())
        {
            Renderer::Submit([]() {
                glStencilFunc(GL_ALWAYS, 1, 0xff);
                glStencilMask(0);
                glLineWidth(3);
                glEnable(GL_LINE_SMOOTH);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDisable(GL_DEPTH_TEST);
            });

            {
                Ref<Mesh> boundMesh = nullptr;
                m_ColliderMaterial->Bind();

                for (auto& dc : debugList)
                {
                    if (!dc.Mesh) continue;

                    if (dc.Mesh != boundMesh)
                    {
                        dc.Mesh->m_VertexBuffer->Bind();
                        dc.Mesh->m_Pipeline->Bind();
                        dc.Mesh->m_IndexBuffer->Bind();
                        boundMesh = dc.Mesh;
                    }

                    m_ObjectUBO.SetModel(dc.Transform);
                    m_ObjectUBO.Upload();
                    m_ObjectUBO.Bind();

                    auto& submesh = dc.Mesh->m_Submeshes[dc.SubmeshIndex];
                    Renderer::DrawIndexedBaseVertex(
                        submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                }
            }

            Renderer::Submit([]() {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glStencilMask(0xff);
                glStencilFunc(GL_ALWAYS, 1, 0xff);
                glEnable(GL_DEPTH_TEST);
                glDisable(GL_STENCIL_TEST);
            });
        }

        // Grid
        if (GetOptions().ShowGrid)
        {
            Renderer::SubmitQuad(m_GridMaterial,
                glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(16.0f)));
        }

        Renderer::EndRenderPass();
    }

    void RenderPipeline::CompositePass()
    {
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(m_CompositePass);
        m_GeoPass->GetSpecification().TargetFramebuffer->BindTexture(Config::PRISM_GEOMETRY_PASS_TEXTURE);

        float exposure = 0.8f;
        m_CompositeMaterial->SetFloat("u_Exposure", exposure);
        m_CompositeMaterial->SetInt("Prism_GeometryPassTextureSamples",
            (int)m_GeoPass->GetSpecification().TargetFramebuffer->GetSpecification().Samples);
        m_CompositeMaterial->Bind();

        Renderer::SubmitFullscreenQuad(nullptr);
        Renderer::EndRenderPass();
    }

    std::pair<Ref<TextureCube>, Ref<TextureCube>>
    RenderPipeline::CreateEnvironmentMap(const std::string& filepath)
    {
        PR_PROFILE_FUNCTION();
        const uint32_t cubemapSize = 2048;
        const uint32_t irradianceMapSize = 32;

        Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
        if (!s_EnvironmentShader)
            s_EnvironmentShader = ComputeShader::Create("Assets/Shaders/Environment.compute");
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

    Environment Environment::Load(const std::string& filepath)
    {
        auto [radiance, irradiance] = RenderPipeline::CreateEnvironmentMap(filepath);
        return { filepath, radiance, irradiance };
    }
}
