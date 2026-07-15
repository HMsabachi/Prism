#include "prpch.h"
#include "RenderPipeline.h"

#include "Prism/Renderer/RenderPass.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Renderer/VertexInput.h"
#include "Prism/Renderer/Buffer/VertexBuffer.h"
#include "Prism/Renderer/Buffer/IndexBuffer.h"
#include "Prism/Renderer/Buffer/Framebuffer.h"
#include "Prism/Renderer/Shader/PrismShader.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "Prism/ShaderCompiler/PrismBindings.h"
#include "Prism/Renderer/Camera/Camera.h"

#include "Prism/Core/Hash.h"

#include <glad/glad.h>

namespace Prism
{
    constexpr uint64_t SHADER_TAG_KEY_LIGHT_MODE = Hash::GenerateFNVHash64("LightMode");
    constexpr uint64_t SHADER_TAG_VALUE_FORWARD_BASE = Hash::GenerateFNVHash64("ForwardBase");
    constexpr uint64_t SHADER_TAG_VALUE_SHADOW_CASTER = Hash::GenerateFNVHash64("ShadowCaster");

    void RenderPipeline::Initialize(uint32_t viewportWidth, uint32_t viewportHeight)
    {
        m_FrameUBO.Init();
        m_ObjectUBO.Init();

        FramebufferSpecification geoFBSpec;
        geoFBSpec.Width = viewportWidth;
        geoFBSpec.Height = viewportHeight;
        geoFBSpec.Attachments = { FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::Depth };
        geoFBSpec.Samples = 8;
        geoFBSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification geoRPSpec;
        geoRPSpec.TargetFramebuffer = Framebuffer::Create(geoFBSpec);
        m_GeoPass = RenderPass::Create(geoRPSpec);

        FramebufferSpecification compFBSpec;
        compFBSpec.Width = viewportWidth;
        compFBSpec.Height = viewportHeight;
        compFBSpec.Attachments = { FramebufferTextureFormat::RGBA8 };
        compFBSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification compRPSpec;
        compRPSpec.TargetFramebuffer = Framebuffer::Create(compFBSpec);
        m_CompositePass = RenderPass::Create(compRPSpec);

        m_CompositeShader = AssetManager::GetShaderLibrary()->Get("Custom/SceneComposite");
        m_CompositeMaterial = Material::Create(m_CompositeShader->Handle);
        m_BRDFLUT = Texture2D::Create("Assets/Textures/BRDF_LUT.tga");

        auto gridShader = AssetManager::GetShaderLibrary()->Get("Custom/Grid");
        m_GridMaterial = Material::Create(gridShader->Handle);
        m_GridMaterial->SetFloat("u_Scale", 16.025f);
        m_GridMaterial->SetFloat("u_Res", 0.025f);

        auto outlineShader = AssetManager::GetShaderLibrary()->Get("Standard/Outline");
        m_OutlineMaterial = Material::Create(outlineShader->Handle);
        m_OutlineAnimMaterial = Material::Create(outlineShader->Handle);
        m_OutlineAnimMaterial->SetKeyword("SKINNED", true);

        auto colliderShader = AssetManager::GetShaderLibrary()->Get("Debug/Collider");
        m_ColliderMaterial = Material::Create(colliderShader->Handle);

        FramebufferSpecification shadowFBSpec;
        shadowFBSpec.Width = SHADOW_MAP_SIZE;
        shadowFBSpec.Height = SHADOW_MAP_SIZE;
        shadowFBSpec.Attachments = { FramebufferTextureFormat::DEPTH32F };
        shadowFBSpec.NoResize = true;
        shadowFBSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < 4; i++)
        {
            RenderPassSpecification shadowRPSpec;
            shadowRPSpec.TargetFramebuffer = Framebuffer::Create(shadowFBSpec);
            m_ShadowPasses[i] = RenderPass::Create(shadowRPSpec);
        }

        // Bloom Blur
        FramebufferSpecification bloomBlurFBSpec;
        bloomBlurFBSpec.Attachments = { FramebufferTextureFormat::RGBA16F };
        bloomBlurFBSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

        RenderPassSpecification bloomBlurRPSpec;
        bloomBlurRPSpec.TargetFramebuffer = Framebuffer::Create(bloomBlurFBSpec);
        m_BloomBlurPass[0] = RenderPass::Create(bloomBlurRPSpec);
        bloomBlurRPSpec.TargetFramebuffer = Framebuffer::Create(bloomBlurFBSpec);
        m_BloomBlurPass[1] = RenderPass::Create(bloomBlurRPSpec);

        auto bloomBlurShader = AssetManager::GetShaderLibrary()->Get("PostProcess/BloomBlur");
        if (bloomBlurShader)
            m_BloomBlurMaterial = Material::Create(bloomBlurShader->Handle);

        // Bloom Blend
        FramebufferSpecification bloomBlendFBSpec;
        bloomBlendFBSpec.Attachments = { FramebufferTextureFormat::RGBA8 };
        bloomBlendFBSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification bloomBlendRPSpec;
        bloomBlendRPSpec.TargetFramebuffer = Framebuffer::Create(bloomBlendFBSpec);
        m_BloomBlendPass = RenderPass::Create(bloomBlendRPSpec);

        auto bloomBlendShader = AssetManager::GetShaderLibrary()->Get("PostProcess/BloomBlend");
        if (bloomBlendShader)
            m_BloomBlendMaterial = Material::Create(bloomBlendShader->Handle);

        CreateFullscreenQuad();
    }

    void RenderPipeline::Shutdown()
    {
        m_GeoPass.Reset();
        m_CompositePass.Reset();
        for (int i = 0; i < 4; i++)
            m_ShadowPasses[i].Reset();
        m_BloomBlurPass[0].Reset();
        m_BloomBlurPass[1].Reset();
        m_BloomBlendPass.Reset();
        m_BloomBlurMaterial.Reset();
        m_BloomBlendMaterial.Reset();

        m_FullscreenQuadVB.Reset();
        m_FullscreenQuadIB.Reset();
        m_FullscreenQuadPipeline.Reset();
    }

    void RenderPipeline::Resize(uint32_t width, uint32_t height)
    {
        m_GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
        m_CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
    }

    void RenderPipeline::Execute(const FrameSnapshot& snapshot)
    {
        std::vector<DrawCommand> sortedDrawList = snapshot.DrawList;
        std::sort(sortedDrawList.begin(), sortedDrawList.end(),
            [](auto& a, auto& b) { return a.SortKey < b.SortKey; });

        const auto& config = snapshot.Config;
        bool castShadows = config.ShadowsEnabled && !snapshot.DrawList.empty()
            && config.LightEnvironment.DirectionalLights[0].CastShadows;
        if (castShadows)
            UpdateShadowData(snapshot);
        {
            auto& dl = config.LightEnvironment.DirectionalLights[0];
            m_FrameUBO.SetShadowData(dl.LightSize, config.MaxShadowDistance, 25.0f, 0.0f);
        }
        BeginFrame(snapshot);
        if (castShadows)
            ShadowPass(snapshot.ShadowDrawList);
        GeometryPass(config, sortedDrawList, snapshot.SelectedDrawList, snapshot.DebugDrawList);
        // TODO: BloomBlurPass() — need MSAA resolve on Attachment1 before reading as sampler2D
        // if (config.EnableBloom) BloomBlurPass();
        CompositePass();
    }

#pragma region Passes

    void RenderPipeline::ShadowPass(const std::vector<DrawCommand>& drawList)
    {
        PR_PROFILE_FUNCTION();
        if (drawList.empty()) return;

        for (uint32_t cascade = 0; cascade < 4; cascade++)
        {
            Renderer::BeginRenderPass(m_ShadowPasses[cascade]);

            for (auto& dc : drawList)
            {
                int32_t shadowPass = dc.Material->GetShader()->FindPassByTag(SHADER_TAG_KEY_LIGHT_MODE, SHADER_TAG_VALUE_SHADOW_CASTER);
                if (shadowPass < 0) continue;

                m_ObjectUBO.SetModel(dc.Transform);
                m_ObjectUBO.SetShadowPassIndex((int)cascade);
                if (dc.Mesh->IsAnimated())
                    m_ObjectUBO.SetBones(dc.Mesh->m_BoneTransforms.data(),
                        (uint32_t)dc.Mesh->m_BoneTransforms.size());
                m_ObjectUBO.Upload();
                m_ObjectUBO.Bind();

                Renderer::RenderMesh(dc.Mesh->GetVertexInput(), dc.Mesh, dc.Material,
                    dc.SubmeshIndex, dc.Transform, (uint32_t)shadowPass);
            }

            Renderer::EndRenderPass();
        }
    }

    void RenderPipeline::GeometryPass(
        const RenderConfig& config,
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

        DrawFullscreen(config.SkyboxMaterial);

        for (int i = 0; i < 4; i++)
            m_ShadowPasses[i]->GetSpecification().TargetFramebuffer->BindDepthTexture(Config::PRISM_SHADOW_MAP0 + i);

        if (!drawList.empty())
        {
            for (auto& dc : drawList)
            {
                auto& material = dc.Material;
                int32_t forwardBasePass = material->GetShader()->FindPassByTag(SHADER_TAG_KEY_LIGHT_MODE, SHADER_TAG_VALUE_FORWARD_BASE);

                m_ObjectUBO.SetModel(dc.Transform);
                if (dc.Mesh->IsAnimated())
                    m_ObjectUBO.SetBones(dc.Mesh->m_BoneTransforms.data(),
                        (uint32_t)dc.Mesh->m_BoneTransforms.size());
                m_ObjectUBO.Upload();
                m_ObjectUBO.Bind();

                Renderer::RenderMesh(dc.Mesh->GetVertexInput(), dc.Mesh, material,
                    dc.SubmeshIndex, dc.Transform, (uint32_t)forwardBasePass);
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
                    auto material = dc.Material;

                    if (material != boundMaterial)
                    {
                        material->Bind();
                        boundMaterial = material;
                    }
                    if (dc.Mesh != boundMesh)
                    {
                        dc.Mesh->m_VertexBuffer->Bind();
                        dc.Mesh->m_VertexInput->Bind();
                        dc.Mesh->m_IndexBuffer->Bind();
                        boundMesh = dc.Mesh;
                    }

                    m_ObjectUBO.SetModel(dc.Transform);
                    if (dc.Mesh->IsAnimated())
                        m_ObjectUBO.SetBones(dc.Mesh->m_BoneTransforms.data(),
                            (uint32_t)dc.Mesh->m_BoneTransforms.size());
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
                Ref<Material> boundMaterial;

                for (auto& dc : selectedList)
                {
                    auto material = dc.Mesh->IsAnimated() ? m_OutlineAnimMaterial : m_OutlineMaterial;
                    if (material != boundMaterial)
                    {
                        material->Bind();
                        boundMaterial = material;
                    }

                    if (dc.Mesh != boundMesh)
                    {
                        dc.Mesh->m_VertexBuffer->Bind();
                        dc.Mesh->m_VertexInput->Bind();
                        dc.Mesh->m_IndexBuffer->Bind();
                        boundMesh = dc.Mesh;
                    }

                    m_ObjectUBO.SetModel(dc.Transform);
                    if (dc.Mesh->IsAnimated())
                        m_ObjectUBO.SetBones(dc.Mesh->m_BoneTransforms.data(),
                            (uint32_t)dc.Mesh->m_BoneTransforms.size());
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
                        dc.Mesh->m_VertexInput->Bind();
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
                glEnable(GL_DEPTH_TEST);
            });
        }

        // Grid
        if (GetOptions().ShowGrid)
        {
            DrawQuad(m_GridMaterial,
                glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(16.0f)));
        }

        Renderer::EndRenderPass();
    }

    void RenderPipeline::CompositePass()
    {
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(m_CompositePass);
        m_GeoPass->GetSpecification().TargetFramebuffer->BindTexture(0, Config::PRISM_GEOMETRY_PASS_TEXTURE);

        float exposure = 0.8f;
        m_CompositeMaterial->SetFloat("u_Exposure", exposure);
        m_CompositeMaterial->SetInt("Prism_GeometryPassTextureSamples",
            (int)m_GeoPass->GetSpecification().TargetFramebuffer->GetSpecification().Samples);
        m_CompositeMaterial->Bind();

        DrawFullscreen(nullptr);
        Renderer::EndRenderPass();
    }

    void RenderPipeline::BloomBlurPass()
    {
        PR_PROFILE_FUNCTION();
        int amount = 10;
        int index = 0;

        for (int i = 0; i < amount; i++)
        {
            index = i % 2;
            Renderer::BeginRenderPass(m_BloomBlurPass[index]);
            m_BloomBlurMaterial->SetBool("u_Horizontal", index == 0);
            m_BloomBlurMaterial->Bind();
            if (i > 0)
            {
                auto fb = m_BloomBlurPass[1 - index]->GetSpecification().TargetFramebuffer;
                fb->BindTexture(0, Config::PRISM_GEOMETRY_PASS_TEXTURE);
            }
            else
            {
                auto fb = m_GeoPass->GetSpecification().TargetFramebuffer;
                fb->BindTexture(1, Config::PRISM_GEOMETRY_PASS_TEXTURE);
            }
            DrawFullscreen(nullptr);
            Renderer::EndRenderPass();
        }
    }

    void RenderPipeline::BloomBlendPass()
    {
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(m_BloomBlendPass);
        m_BloomBlendMaterial->SetFloat("u_Exposure", 0.8f);
        m_BloomBlendMaterial->SetBool("u_EnableBloom", true);
        m_BloomBlendMaterial->Bind();

        m_GeoPass->GetSpecification().TargetFramebuffer->BindTexture(0, Config::PRISM_GEOMETRY_PASS_TEXTURE);
        m_BloomBlurPass[1]->GetSpecification().TargetFramebuffer->BindTexture(0, Config::PRISM_BLOOM_TEXTURE);

        DrawFullscreen(nullptr);
        Renderer::EndRenderPass();
    }
#pragma endregion

#pragma region Tool

    void RenderPipeline::CreateFullscreenQuad()
    {
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

        VertexInputSpecification pipelineSpec;
        pipelineSpec.Layout = {
            { ShaderDataType::Float3, "a_Position",  VertexSemantic::Position },
            { ShaderDataType::Float2, "a_TexCoord",  VertexSemantic::TexCoord0 }
        };
        m_FullscreenQuadPipeline = VertexInput::Create(pipelineSpec);
        m_FullscreenQuadVB = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));

        uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
        m_FullscreenQuadIB = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
    }

    void RenderPipeline::DrawFullscreen(const Ref<Material>& material)
    {
        if (material)
            material->Bind();
        m_FullscreenQuadVB->Bind();
        m_FullscreenQuadPipeline->Bind();
        m_FullscreenQuadIB->Bind();
        Renderer::DrawIndexed(6, PrimitiveType::Triangles, true);
    }

    void RenderPipeline::DrawQuad(const Ref<Material>& material, const glm::mat4& transform)
    {
        if (material)
        {
            material->Bind();
            m_ObjectUBO.SetModel(transform);
            m_ObjectUBO.Upload();
            m_ObjectUBO.Bind();
        }
        m_FullscreenQuadVB->Bind();
        m_FullscreenQuadPipeline->Bind();
        m_FullscreenQuadIB->Bind();
        Renderer::DrawIndexed(6, PrimitiveType::Triangles, true);
    }

    void RenderPipeline::BeginFrame(const FrameSnapshot& snapshot)
    {
        auto& cam = snapshot.Camera;
        const auto& config = snapshot.Config;
        m_FrameUBO.SetViewProjection(cam.Projection.GetProjectionMatrix() * cam.ViewMatrix);
        m_FrameUBO.SetView(cam.ViewMatrix);
        m_FrameUBO.SetProjection(cam.Projection.GetProjectionMatrix());
        m_FrameUBO.SetCameraPosition(glm::inverse(cam.ViewMatrix)[3]);
        m_FrameUBO.SetTime(Time::GetTime(), Time::GetDeltaTime());
        auto directionalLight = config.LightEnvironment.DirectionalLights[0];
        m_FrameUBO.SetLight(0, directionalLight.Direction,
            directionalLight.Radiance, directionalLight.Multiplier);
        m_FrameUBO.Upload();
        m_FrameUBO.Bind();
        if (config.SceneEnvironment && config.SceneEnvironment->RadianceMap && config.SceneEnvironment->IrradianceMap)
        {
            config.SceneEnvironment->RadianceMap->Bind(Config::PRISM_ENV_RADIANCE);
            config.SceneEnvironment->IrradianceMap->Bind(Config::PRISM_ENV_IRRADIANCE);
        }
        m_BRDFLUT->Bind(Config::PRISM_ENV_BRDF_LUT);
    }

    void RenderPipeline::UpdateShadowData(const FrameSnapshot& snapshot)
    {
        auto& camera = snapshot.Camera;
        const auto& config = snapshot.Config;
        auto directionalLight = config.LightEnvironment.DirectionalLights[0];
        uint32_t cascadeCount = glm::clamp(config.CascadeCount, 1u, 4u);

        auto& proj = camera.Projection.GetProjectionMatrix();
        float f = proj[1][1];
        float fov = 2.0f * atan(1.0f / f);
        float aspect = proj[1][1] / proj[0][0];
        float nearClip = proj[3][2] / (proj[2][2] - 1.0f);
        float farClip = proj[3][2] / (proj[2][2] + 1.0f);
        farClip = glm::min(farClip, config.MaxShadowDistance);

        float splits[4] = {};
        float splitLambda = 0.82f;
        for (uint32_t i = 0; i < cascadeCount; i++)
        {
            float fraction = (float)(i + 1) / (float)cascadeCount;
            float logSplit = nearClip * pow(farClip / nearClip, fraction);
            float uniSplit = nearClip + (farClip - nearClip) * fraction;
            splits[i] = logSplit * splitLambda + uniSplit * (1.0f - splitLambda);
        }

        m_CascadeSplits = glm::vec4(splits[0], splits[1], splits[2], splits[3]);
        m_FrameUBO.SetShadowParams(config.ShadowBias, config.ShadowNormalBias,
            (float)cascadeCount, directionalLight.SoftShadows ? 1.0f : 0.0f);

        glm::mat4 invView = glm::inverse(camera.ViewMatrix);
        glm::vec3 lightDir = glm::normalize(directionalLight.Direction);
        glm::vec3 up = glm::abs(lightDir.y) < 0.99f
            ? glm::vec3(0.0f, 1.0f, 0.0f)
            : glm::vec3(1.0f, 0.0f, 0.0f);

        glm::vec3 camPos = invView[3];
        glm::vec3 camForward = -glm::normalize(glm::vec3(invView[2]));
        float midDist = (nearClip + farClip) * 0.5f;
        glm::vec3 frustumCenter = camPos + camForward * midDist;
        float lightDist = farClip * 2.0f + 200.0f;
        glm::vec3 lightEye = frustumCenter - lightDir * lightDist;
        glm::mat4 lightView = glm::lookAt(lightEye, frustumCenter, up);

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
    }

#pragma endregion
}
