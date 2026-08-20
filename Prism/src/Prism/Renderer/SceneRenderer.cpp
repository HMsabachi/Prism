#include "prpch.h"
#include "SceneRenderer.h"

#include "Prism/Renderer/RenderPass.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Renderer/VertexInput.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Prism/Renderer/Buffer/VertexBuffer.h"
#include "Prism/Renderer/Buffer/IndexBuffer.h"
#include "Prism/Renderer/Buffer/Framebuffer.h"
#include "Prism/Renderer/Shader/PrismShader.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "Prism/ShaderCompiler/PrismBindings.h"
#include "Prism/Renderer/Camera/Camera.h"

#include "Prism/ImGui/ImGui.h"
#include "Prism/Core/LanguageManager.h"

#include "Prism/Core/Hash.h"

namespace Prism
{
    constexpr uint64_t SHADER_TAG_KEY_LIGHT_MODE = Hash::GenerateFNVHash64("LightMode");
    constexpr uint64_t SHADER_TAG_VALUE_FORWARD_BASE = Hash::GenerateFNVHash64("ForwardBase");
    constexpr uint64_t SHADER_TAG_VALUE_SHADOW_CASTER = Hash::GenerateFNVHash64("ShadowCaster");

    static std::vector<std::thread> s_ThreadPool;

    SceneRenderer* SceneRenderer::s_Instance = nullptr;

    SceneRenderer::SceneRenderer() { s_Instance = this; }
    SceneRenderer::~SceneRenderer() { if (s_Instance == this) s_Instance = nullptr; }

    SceneRenderer& SceneRenderer::Get()
    {
        PR_CORE_ASSERT(s_Instance, "SceneRenderer not initialized!");
        return *s_Instance;
    }

    Ref<Image2D> SceneRenderer::GetFinalImage() const
    {
        return m_CompositePass ? m_CompositePass->GetSpecification().TargetFramebuffer->GetImage() : nullptr;
    }

    void SceneRenderer::Initialize(uint32_t viewportWidth, uint32_t viewportHeight)
    {
        m_FrameUBO = UniformBuffer::Create(sizeof(FrameData));
        m_ObjectUBO = UniformBuffer::Create(sizeof(ObjectData));

        FramebufferSpecification geoFBSpec;
        geoFBSpec.Width = viewportWidth;
        geoFBSpec.Height = viewportHeight;
        geoFBSpec.Attachments = { ImageFormat::RGBA16F, ImageFormat::RGBA16F, ImageFormat::Depth };
        geoFBSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification geoRPSpec;
        geoRPSpec.TargetFramebuffer = Framebuffer::Create(geoFBSpec);
        m_GeoPass = RenderPass::Create(geoRPSpec);

        FramebufferSpecification idFBSpec;
        idFBSpec.Width = viewportWidth;
        idFBSpec.Height = viewportHeight;
        idFBSpec.Attachments = { ImageFormat::RGBA16F };
        idFBSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

        RenderPassSpecification idRPSpec;
        idRPSpec.TargetFramebuffer = Framebuffer::Create(idFBSpec);
        m_IDPass = RenderPass::Create(idRPSpec);

        FramebufferSpecification compFBSpec;
        compFBSpec.Width = viewportWidth;
        compFBSpec.Height = viewportHeight;
        compFBSpec.Attachments = { ImageFormat::RGBA };
        compFBSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification compRPSpec;
        compRPSpec.TargetFramebuffer = Framebuffer::Create(compFBSpec);
        m_CompositePass = RenderPass::Create(compRPSpec);

        m_BRDFLUT = Texture2D::Create("Assets/Textures/BRDF_LUT.tga");

        auto compositeShader = AssetManager::GetShaderLibrary()->Get("Custom/SceneComposite");
        m_CompositeMaterial = Material::Create(compositeShader->Handle);

        auto gridShader = AssetManager::GetShaderLibrary()->Get("Custom/Grid");
        m_GridMaterial = Material::Create(gridShader->Handle);
        m_GridMaterial->SetFloat("u_Scale", 16.025f);
        m_GridMaterial->SetFloat("u_Res", 0.025f);

        auto idShader = AssetManager::GetShaderLibrary()->Get("Standard/ObjectID");
        m_IDMaterial = Material::Create(idShader->Handle);
        m_IDAnimMaterial = Material::Create(idShader->Handle);
        m_IDAnimMaterial->SetKeyword("SKINNED", true);

        auto colliderShader = AssetManager::GetShaderLibrary()->Get("Debug/Collider");
        m_ColliderMaterial = Material::Create(colliderShader->Handle);

        FramebufferSpecification shadowFBSpec;
        shadowFBSpec.Width = SHADOW_MAP_SIZE;
        shadowFBSpec.Height = SHADOW_MAP_SIZE;
        shadowFBSpec.Attachments = { ImageFormat::DEPTH32F };
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
        bloomBlurFBSpec.Attachments = { ImageFormat::RGBA16F };
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
        bloomBlendFBSpec.Attachments = { ImageFormat::RGBA };
        bloomBlendFBSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification bloomBlendRPSpec;
        bloomBlendRPSpec.TargetFramebuffer = Framebuffer::Create(bloomBlendFBSpec);
        m_BloomBlendPass = RenderPass::Create(bloomBlendRPSpec);

        auto bloomBlendShader = AssetManager::GetShaderLibrary()->Get("PostProcess/BloomBlend");
        if (bloomBlendShader)
            m_BloomBlendMaterial = Material::Create(bloomBlendShader->Handle);

        CreateFullscreenQuad();
    }

    void SceneRenderer::Shutdown()
    {
        m_GeoPass.Reset();
        m_IDPass.Reset();
        m_CompositePass.Reset();
        for (int i = 0; i < 4; i++)
            m_ShadowPasses[i].Reset();
        m_BloomBlurPass[0].Reset();
        m_BloomBlurPass[1].Reset();
        m_BloomBlendPass.Reset();
        m_BloomBlurMaterial.Reset();
        m_BloomBlendMaterial.Reset();

        m_FullscreenQuadPipeline.Reset();
    }


    void SceneRenderer::OnImGuiRender()
    {
        ImGui::Begin("SceneRenderer");
        if (UI::BeginTreeNode(TR("Shadow Pass"), false))
        {
            static int cascadeIndex = 0;
            UI::BeginPropertyGrid();
            UI::PropertySlider("Cascade Index", cascadeIndex, 0, 3);
            UI::EndPropertyGrid();
            Ref<Image2D> depthImage = m_ShadowPasses[cascadeIndex]->GetSpecification().TargetFramebuffer->GetDepthImage();
            float size = ImGui::GetContentRegionAvail().x;
            UI::Image(depthImage, { size, size }, { 0, 1 }, { 1, 0 });
            UI::EndTreeNode();
        }
        if (UI::BeginTreeNode(TR("Geometry Pass"), false))
        {
            Ref<Image2D> colorImage = m_GeoPass->GetSpecification().TargetFramebuffer->GetImage(0);
            Ref<Image2D> bloomImage = m_GeoPass->GetSpecification().TargetFramebuffer->GetImage(1);
            Ref<Image2D> depthImage = m_GeoPass->GetSpecification().TargetFramebuffer->GetDepthImage();
            float size = ImGui::GetContentRegionAvail().x;
            UI::Image(colorImage, { size, size }, { 0, 1 }, { 1, 0 });
            UI::Image(bloomImage, { size, size }, { 0, 1 }, { 1, 0 });
            UI::Image(depthImage, { size, size }, { 0, 1 }, { 1, 0 });
            UI::EndTreeNode();
        }
        if (UI::BeginTreeNode(TR("ID Pass"), false))
        {
            Ref<Image2D> idImage = m_IDPass->GetSpecification().TargetFramebuffer->GetImage();
            float size = ImGui::GetContentRegionAvail().x;
            UI::Image(idImage, { size, size }, { 0, 1 }, { 1, 0 });
            UI::EndTreeNode();
        }
        if (UI::BeginTreeNode(TR("Final Image"), false))
        {
            Ref<Image2D> finalImage = m_CompositePass->GetSpecification().TargetFramebuffer->GetImage();
            float size = ImGui::GetContentRegionAvail().x;
            UI::Image(finalImage, { size, size }, { 0, 1 }, { 1, 0 });
            UI::EndTreeNode();
        }
        ImGui::End();
    }

    void SceneRenderer::RT_Resize(uint32_t width, uint32_t height)
    {
        m_GeoPass->GetSpecification().TargetFramebuffer->RT_Resize(width, height);
        m_IDPass->GetSpecification().TargetFramebuffer->RT_Resize(width, height);
        m_CompositePass->GetSpecification().TargetFramebuffer->RT_Resize(width, height);
    }

    void SceneRenderer::Execute(const FrameSnapshot& snapshot)
    {
        ExecuteImpt(snapshot);
        return;
    }
    void SceneRenderer::ExecuteImpt(const FrameSnapshot& snapshot)
    {
        std::vector<DrawCommand> sortedDrawList = snapshot.DrawList;
        std::sort(sortedDrawList.begin(), sortedDrawList.end(), [](auto& a, auto& b) { return a.SortKey < b.SortKey; });

        const auto& config = snapshot.Config;
        bool castShadows = config.ShadowsEnabled && !snapshot.DrawList.empty() && config.LightEnvironment.DirectionalLights[0].CastShadows;
        if (castShadows) UpdateShadowData(snapshot);

        auto& dl = config.LightEnvironment.DirectionalLights[0];
        m_FrameData.ShadowData = { dl.LightSize, config.MaxShadowDistance, 25.0f, 0.0f };
        BeginFrame(snapshot);
        if (castShadows) ShadowPass(snapshot.ShadowDrawList);
        GeometryPass(config, sortedDrawList, snapshot.SelectedDrawList, snapshot.DebugDrawList);
        IDPass(snapshot.SelectedDrawList);
        // TODO: BloomBlurPass() — need MSAA resolve on Attachment1 before reading as sampler2D
        // if (config.EnableBloom) BloomBlurPass();
        CompositePass();
    }

#pragma region Passes

    void SceneRenderer::ShadowPass(const std::vector<DrawCommand>& drawList)
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

                m_ObjectData.Model = dc.Transform;
                m_ObjectData.Reserved.x = static_cast<float>(cascade);
                if (dc.Mesh->IsAnimated())
                    SetObjectBones(dc.Mesh->m_BoneTransforms.data(), (uint32_t)dc.Mesh->m_BoneTransforms.size());
                UploadObjectUBO();
                Renderer::SetUniformBuffer(Config::PRISM_SET_TRANSFORMS, 0, m_ObjectUBO);

                Renderer::RenderMesh(dc.Mesh, dc.Material, dc.SubmeshIndex, dc.Transform, (uint32_t)shadowPass);
            }

            Renderer::EndRenderPass();
        }
    }

    void SceneRenderer::GeometryPass(
        const RenderConfig& config,
        const std::vector<DrawCommand>& drawList,
        const std::vector<DrawCommand>& selectedList,
        const std::vector<DrawCommand>& debugList)
    {
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(m_GeoPass);

        DrawFullscreen(config.SkyboxMaterial);

        for (int i = 0; i < 4; i++)
            Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_GEOMETRY_SHADOW_MAP0_SLOT + i,
                m_ShadowPasses[i]->GetSpecification().TargetFramebuffer->GetDepthImage());

        if (!drawList.empty())
        {
            for (auto& dc : drawList)
            {
                auto& material = dc.Material;
                int32_t forwardBasePass = material->GetShader()->FindPassByTag(SHADER_TAG_KEY_LIGHT_MODE, SHADER_TAG_VALUE_FORWARD_BASE);

                m_ObjectData.Model = dc.Transform;
                if (dc.Mesh->IsAnimated())
                    SetObjectBones(dc.Mesh->m_BoneTransforms.data(),
                        (uint32_t)dc.Mesh->m_BoneTransforms.size());
                UploadObjectUBO();
                Renderer::SetUniformBuffer(Config::PRISM_SET_TRANSFORMS, 0, m_ObjectUBO);

                Renderer::RenderMesh(dc.Mesh, material, dc.SubmeshIndex, dc.Transform, (uint32_t)forwardBasePass);
            }
        }

        if (!selectedList.empty())
        {
            for (auto& dc : selectedList)
            {
                m_ObjectData.Model = dc.Transform;
                if (dc.Mesh->IsAnimated())
                    SetObjectBones(dc.Mesh->m_BoneTransforms.data(),
                        (uint32_t)dc.Mesh->m_BoneTransforms.size());
                UploadObjectUBO();
                Renderer::SetUniformBuffer(Config::PRISM_SET_TRANSFORMS, 0, m_ObjectUBO);

                Renderer::RenderMesh(dc.Mesh, dc.Material, dc.SubmeshIndex, dc.Transform, 0);
            }
        }

        if (!debugList.empty())
        {
            for (auto& dc : debugList)
            {
                if (!dc.Mesh) continue;

                m_ObjectData.Model = dc.Transform;
                UploadObjectUBO();
                Renderer::SetUniformBuffer(Config::PRISM_SET_TRANSFORMS, 0, m_ObjectUBO);

                Renderer::RenderMesh(dc.Mesh, m_ColliderMaterial, dc.SubmeshIndex, dc.Transform, 0);
            }
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

    void SceneRenderer::IDPass(const std::vector<DrawCommand>& selectedList)
    {
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(m_IDPass);

        uint32_t objectID = 1;
        for (auto& dc : selectedList)
        {
            m_ObjectData.Model = dc.Transform;
            m_ObjectData.Reserved.y = static_cast<float>(objectID++);
            if (dc.Mesh->IsAnimated())
                SetObjectBones(dc.Mesh->m_BoneTransforms.data(),
                    (uint32_t)dc.Mesh->m_BoneTransforms.size());
            UploadObjectUBO();
            Renderer::SetUniformBuffer(Config::PRISM_SET_TRANSFORMS, 0, m_ObjectUBO);

            Ref<Material> material = dc.Mesh->IsAnimated() ? m_IDAnimMaterial : m_IDMaterial;
            Renderer::RenderMesh(dc.Mesh, material, dc.SubmeshIndex, dc.Transform, 0);
        }

        Renderer::EndRenderPass();
    }

    void SceneRenderer::CompositePass()
    {
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(m_CompositePass);
        Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_COMPOSITE_GEOMETRY_COLOR_SLOT,
            m_GeoPass->GetSpecification().TargetFramebuffer->GetImage(0));
        Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_COMPOSITE_OBJECT_ID_SLOT,
            m_IDPass->GetSpecification().TargetFramebuffer->GetImage(0));

        float exposure = 0.8f;
        m_CompositeMaterial->SetFloat("u_Exposure", exposure);

        DrawFullscreen(m_CompositeMaterial);
        Renderer::EndRenderPass();
    }

    void SceneRenderer::BloomBlurPass()
    {
        PR_PROFILE_FUNCTION();
        int amount = 10;
        int index = 0;

        for (int i = 0; i < amount; i++)
        {
            index = i % 2;
            Renderer::BeginRenderPass(m_BloomBlurPass[index]);
            m_BloomBlurMaterial->SetBool("u_Horizontal", index == 0);
            if (i > 0)
            {
                auto fb = m_BloomBlurPass[1 - index]->GetSpecification().TargetFramebuffer;
                Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_BLOOM_BLUR_INPUT_SLOT, fb->GetImage(0));
            }
            else
            {
                auto fb = m_GeoPass->GetSpecification().TargetFramebuffer;
                Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_BLOOM_BLUR_INPUT_SLOT, fb->GetImage(1));
            }
            DrawFullscreen(m_BloomBlurMaterial);
            Renderer::EndRenderPass();
        }
    }

    void SceneRenderer::BloomBlendPass()
    {
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(m_BloomBlendPass);
        m_BloomBlendMaterial->SetFloat("u_Exposure", 0.8f);
        m_BloomBlendMaterial->SetBool("u_EnableBloom", true);

        Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_BLOOM_BLEND_GEOMETRY_COLOR_SLOT,
            m_GeoPass->GetSpecification().TargetFramebuffer->GetImage(0));
        Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_BLOOM_BLEND_BLOOM_SLOT,
            m_BloomBlurPass[1]->GetSpecification().TargetFramebuffer->GetImage(0));

        DrawFullscreen(m_BloomBlendMaterial);
        Renderer::EndRenderPass();
    }
#pragma endregion

#pragma region Tool

    void SceneRenderer::CreateFullscreenQuad()
    {
        VertexInputSpecification pipelineSpec;
        pipelineSpec.Layout = {
            { ShaderDataType::Float3, "a_Position",  VertexSemantic::Position },
            { ShaderDataType::Float2, "a_TexCoord",  VertexSemantic::TexCoord0 }
        };
        m_FullscreenQuadPipeline = VertexInput::Create(pipelineSpec);
    }

    void SceneRenderer::DrawFullscreen(const Ref<Material>& material)
    {
        Renderer::SubmitFullscreenQuad(m_FullscreenQuadPipeline, material);
    }

    void SceneRenderer::DrawQuad(const Ref<Material>& material, const glm::mat4& transform)
    {
        if (material)
        {
            m_ObjectData.Model = transform;
            UploadObjectUBO();
            Renderer::SetUniformBuffer(Config::PRISM_SET_TRANSFORMS, 0, m_ObjectUBO);
        }
        Renderer::RenderQuad(m_FullscreenQuadPipeline, material, transform);
    }

    void SceneRenderer::BeginFrame(const FrameSnapshot& snapshot)
    {
        auto& cam = snapshot.Camera;
        const auto& config = snapshot.Config;
        m_FrameData.ViewProjection = cam.Projection.GetProjectionMatrix() * cam.ViewMatrix;
        m_FrameData.InverseViewProjection = glm::inverse(m_FrameData.ViewProjection);
        m_FrameData.View = cam.ViewMatrix;
        m_FrameData.Projection = cam.Projection.GetProjectionMatrix();
        m_FrameData.CameraPosition = glm::vec3(glm::inverse(cam.ViewMatrix)[3]);
        float t = Time::GetTime();
        m_FrameData.Time = glm::vec4(t * 0.2f, t, t * 2.0f, t * 3.0f);
        m_FrameData.DeltaTime = Time::GetDeltaTime();
        auto directionalLight = config.LightEnvironment.DirectionalLights[0];
        m_FrameData.Lights[0].Direction = directionalLight.Direction;
        m_FrameData.Lights[0].Radiance = directionalLight.Radiance;
        m_FrameData.Lights[0].Multiplier = directionalLight.Multiplier;
        m_FrameUBO->SetData(&m_FrameData, sizeof(m_FrameData));
        Renderer::SetUniformBuffer(Config::PRISM_SET_FRAME, 0, m_FrameUBO);
        Renderer::SetSceneEnvironment(config.SceneEnvironment);
        if (config.SceneEnvironment && config.SceneEnvironment->RadianceMap && config.SceneEnvironment->IrradianceMap)
        {
            Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_GEOMETRY_ENV_RADIANCE_SLOT, config.SceneEnvironment->RadianceMap->GetImage());
            Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_GEOMETRY_ENV_IRRADIANCE_SLOT, config.SceneEnvironment->IrradianceMap->GetImage());
        }
        Renderer::SetTexture(Config::PRISM_SET_RENDER_PASS, Config::PRISM_GEOMETRY_ENV_BRDF_LUT_SLOT, m_BRDFLUT->GetImage());
    }

    void SceneRenderer::UpdateShadowData(const FrameSnapshot& snapshot)
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
        m_FrameData.ShadowParams = { config.ShadowBias, config.ShadowNormalBias,
            (float)cascadeCount, directionalLight.SoftShadows ? 1.0f : 0.0f };

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

        for (uint32_t i = 0; i < cascadeCount; i++)
            m_FrameData.ShadowMatrices[i] = m_ShadowMatrices[i];
        m_FrameData.CascadeSplits = m_CascadeSplits;
    }

    void SceneRenderer::SetObjectBones(const glm::mat4* bones, uint32_t count)
    {
        if (count > PRISM_MAX_BONES) count = PRISM_MAX_BONES;
        memcpy(m_ObjectData.Bones, bones, count * sizeof(glm::mat4));
        m_ObjectBonesDirty = true;
    }

    void SceneRenderer::UploadObjectUBO()
    {
        if (m_ObjectBonesDirty)
            m_ObjectUBO->SetData(&m_ObjectData, sizeof(m_ObjectData));
        else
            m_ObjectUBO->SetData(&m_ObjectData, offsetof(ObjectData, Bones));
        m_ObjectBonesDirty = false;
    }

    

#pragma endregion
}
