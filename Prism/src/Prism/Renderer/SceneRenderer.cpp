#include "prpch.h"
#include "SceneRenderer.h"

#include "ComputeShader/ComputeShader.h"

#include "Shader/GlobalUniforms.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Prism/Shader/PSL/PrismBindings.h"
#include "Prism/Renderer/Camera/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Prism
{
    static void UpdateGlobalsUBO();
    static void UpdateShadowData();

    static constexpr uint32_t SHADOW_MAP_SIZE = 2048;

    struct SceneRendererData
    {
        const Scene* ActiveScene = nullptr;
        struct SceneInfo
        {
            SceneRendererCamera SceneCamera;

            // Resources
            Ref<MaterialInstance> SkyboxMaterial;
            Environment SceneEnvironment;
            Light ActiveLight;
            PrismGlobalsUBO SceneUniforms;

        } SceneData;

        Ref<Texture2D> BRDFLUT;
        Ref<PrismShader> CompositeShader;
        Ref<Material> CompositeMaterial;

        Ref<RenderPass> GeoPass;
        Ref<RenderPass> CompositePass;

        struct DrawCommand
        {
            Ref<Mesh> Mesh;
            Ref<MaterialInstance> Material;
            glm::mat4 Transform;
        };
        std::vector<DrawCommand> DrawList;
        std::vector<DrawCommand> SelectedMeshDrawList;
        std::vector<DrawCommand> ColliderDrawList;

        // Grid
        Ref<MaterialInstance> GridMaterial;
        Ref<MaterialInstance> OutlineMaterial;
        Ref<MaterialInstance> ColliderMaterial;

        // Shadow
        Ref<Framebuffer> ShadowFBOs[4];
        Ref<MaterialInstance> ShadowDepthMaterial;

        SceneRendererOptions Options;
    };

    static SceneRendererData s_Data;

    void SceneRenderer::Init()
    {
        FramebufferSpecification geoFramebufferSpec;
        geoFramebufferSpec.Width = 1280;
        geoFramebufferSpec.Height = 720;
        geoFramebufferSpec.Format = FramebufferFormat::RGBA16F;
        geoFramebufferSpec.Samples = 8;
        geoFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification geoRenderPassSpec;
        geoRenderPassSpec.TargetFramebuffer = Prism::Framebuffer::Create(geoFramebufferSpec);
        s_Data.GeoPass = RenderPass::Create(geoRenderPassSpec);

        FramebufferSpecification compFramebufferSpec;
        compFramebufferSpec.Width = 1280;
        compFramebufferSpec.Height = 720;
        compFramebufferSpec.Format = FramebufferFormat::RGBA8;
        compFramebufferSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };

        RenderPassSpecification compRenderPassSpec;
        compRenderPassSpec.TargetFramebuffer = Prism::Framebuffer::Create(compFramebufferSpec);
        s_Data.CompositePass = RenderPass::Create(compRenderPassSpec);

        s_Data.CompositeShader = Renderer::GetShaderLibrary()->Get("Custom/SceneComposite");
        s_Data.CompositeMaterial = Material::Create(s_Data.CompositeShader);
        s_Data.BRDFLUT = Texture2D::Create("Assets/Textures/BRDF_LUT.tga");

        // Grid
        auto gridShader = Renderer::GetShaderLibrary()->Get("Custom/Grid");
        s_Data.GridMaterial = MaterialInstance::Create(Material::Create(gridShader));
        float gridScale = 16.025f, gridSize = 0.025f;
        s_Data.GridMaterial->Set("u_Scale", gridScale);
        s_Data.GridMaterial->Set("u_Res", gridSize);
        // Outline
        auto outlineShader = Renderer::GetShaderLibrary()->Get("Standard/Outline");
        s_Data.OutlineMaterial = MaterialInstance::Create(Material::Create(outlineShader));

        // Collider
        auto colliderShader = Renderer::GetShaderLibrary()->Get("Debug/Collider");
        s_Data.ColliderMaterial = MaterialInstance::Create(Material::Create(colliderShader));

        // Shadow FBOs
        FramebufferSpecification shadowFBOSpec;
        shadowFBOSpec.Width = SHADOW_MAP_SIZE;
        shadowFBOSpec.Height = SHADOW_MAP_SIZE;
        shadowFBOSpec.Format = FramebufferFormat::Depth;
        for (int i = 0; i < 4; i++)
            s_Data.ShadowFBOs[i] = Framebuffer::Create(shadowFBOSpec);

        // Shadow depth material
        auto shadowShader = Renderer::GetShaderLibrary()->Get("Hidden/ShadowDepth");
        s_Data.ShadowDepthMaterial = MaterialInstance::Create(Material::Create(shadowShader));
    }

    void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
    {
        s_Data.GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
        s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
    }

    void SceneRenderer::BeginScene(const Scene* scene, const SceneRendererCamera& camera)
    {
        PR_CORE_ASSERT(!s_Data.ActiveScene, "");

        s_Data.ActiveScene = scene;

        s_Data.SceneData.SceneCamera = camera;
        s_Data.SceneData.SkyboxMaterial = scene->m_SkyboxMaterial;
        s_Data.SceneData.SceneEnvironment = scene->m_Environment;
        s_Data.SceneData.ActiveLight = scene->m_Light;

        if (s_Data.ActiveScene->IsShadowEnabled())
            UpdateShadowData();

        UpdateGlobalsUBO();
    }

    void SceneRenderer::EndScene()
    {
        PR_CORE_ASSERT(s_Data.ActiveScene, "");

        FlushDrawList();

        s_Data.ActiveScene = nullptr;
    }

    void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
    {
        // TODO: Culling, sorting, etc.

        s_Data.DrawList.push_back({ mesh, overrideMaterial, transform });
    }

    void SceneRenderer::SubmitSelectedMesh(Ref<Mesh> mesh, const glm::mat4& transform)
    {
        s_Data.SelectedMeshDrawList.push_back({ mesh, nullptr, transform });
    }

    void SceneRenderer::SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& parentTransform)
    {
        s_Data.ColliderDrawList.push_back({ component.DebugMesh, nullptr, glm::translate(parentTransform, component.Offset) });
    }

    void SceneRenderer::SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& parentTransform)
    {
        s_Data.ColliderDrawList.push_back({ component.DebugMesh, nullptr, parentTransform });
    }

    void SceneRenderer::SubmitColliderMesh(const CapsuleColliderComponent& component, const glm::mat4& parentTransform)
    {
        s_Data.ColliderDrawList.push_back({ component.DebugMesh, nullptr, parentTransform });
    }

    void SceneRenderer::SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& parentTransform)
    {
        s_Data.ColliderDrawList.push_back({ component.ProcessedMesh, nullptr, parentTransform });
    }

    static Ref<ComputeShader> environmentShader;


    std::pair<Ref<TextureCube>, Ref<TextureCube>> SceneRenderer::CreateEnvironmentMap(const std::string& filepath)
    {
        PR_PROFILE_FUNCTION();
        const uint32_t cubemapSize = 2048;
        const uint32_t irradianceMapSize = 32;

        Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
        if (!environmentShader)
            environmentShader = ComputeShader::Create("Assets/Shaders/Environment.compute");
        Ref<Texture2D> envEquirect = Texture2D::Create(filepath);
        PR_CORE_ASSERT(envEquirect->GetFormat() == TextureFormat::Float16, "Texture is not HDR!");

        envEquirect->Bind();
        int toCubeKernel = environmentShader->FindKernel("CSEquirectToCube");
        environmentShader->SetTexture2D(toCubeKernel, "u_EquirectangularTex", envEquirect);
        environmentShader->SetImageCube(toCubeKernel, "o_OutputCube", envUnfiltered);
        environmentShader->Dispatch(toCubeKernel, cubemapSize / 32, cubemapSize / 32, 6);
        //Renderer::MemoryBarriers(MBarrier::ImageAccess | MBarrier::TextureFetch);
        envUnfiltered->GenerateMipMap();

        Ref<TextureCube> envFiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
        envUnfiltered->CopyTo(envFiltered);
        //Renderer::MemoryBarriers(MBarrier::TextureUpdate | MBarrier::ImageAccess);

        int mipFilter = environmentShader->FindKernel("CSMipFilter");
        environmentShader->SetTextureCube(mipFilter, "u_InputCubeMap", envUnfiltered);
        const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
        for (int level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2)
        {
            const uint32_t numGroups = glm::max(1, size / 32);
            environmentShader->SetImageCube(mipFilter, "o_OutputCube", envFiltered, level, true);
            environmentShader->SetFloat(mipFilter,"u_Roughness", level * deltaRoughness);
            environmentShader->Dispatch(mipFilter, numGroups, numGroups, 6);
        }
        //Renderer::MemoryBarriers(MBarrier::ImageAccess | MBarrier::TextureFetch);

        Ref<TextureCube> irradianceMap = TextureCube::Create(TextureFormat::Float16, irradianceMapSize, irradianceMapSize);
        int irradiance = environmentShader->FindKernel("CSIrradiance");
        environmentShader->SetTextureCube(irradiance, "u_InputCubeMap", envFiltered);
        environmentShader->SetImageCube(irradiance, "o_OutputCube", irradianceMap);
        environmentShader->Dispatch(irradiance, irradianceMapSize / 32, irradianceMapSize / 32, 6);
        irradianceMap->GenerateMipMap();

        return { envFiltered, irradianceMap };
    }



    // TODO: 移除这些
#include <glad/glad.h>
    void SceneRenderer::ShadowPass()
    {
        PR_PROFILE_FUNCTION();
        if (s_Data.DrawList.empty())
            return;

        auto& sceneUniforms = s_Data.SceneData.SceneUniforms;
        uint32_t cascadeCount = (uint32_t)sceneUniforms.ShadowParams.z;
        if (cascadeCount == 0 || cascadeCount > 4)
            return;

        Ref<PrismShader> shadowPrismShader = s_Data.ShadowDepthMaterial->GetShader();
        Ref<Shader> shadowProgram = shadowPrismShader->GetPassProgram(0, 0);

        for (uint32_t cascade = 0; cascade < cascadeCount; cascade++)
        {
            s_Data.ShadowFBOs[cascade]->Bind();
            Renderer::Submit([]() { glClear(GL_DEPTH_BUFFER_BIT); });

            s_Data.ShadowDepthMaterial->Bind();
            shadowProgram->SetMat4("u_LightVP", sceneUniforms.ShadowMatrices[cascade]);

            for (auto& dc : s_Data.DrawList)
                Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data.ShadowDepthMaterial);
        }
    }
    void SceneRenderer::GeometryPass()
    {
        PR_PROFILE_FUNCTION();
        //Renderer::GetRenderCommandQueue().ResetSubmitCount();
        // 描边相关
        bool outline = s_Data.SelectedMeshDrawList.size() > 0;
        bool collider = s_Data.ColliderDrawList.size() > 0;
        //outline = false;
        if (outline || collider)
        {
            Renderer::Submit([]()
            {
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            });
        }
        Renderer::BeginRenderPass(s_Data.GeoPass);

        if (outline || collider)
        {
            Renderer::Submit([]()
                {
                    glStencilMask(0);
                });
        }
        // Skybox
        auto skyboxShader = s_Data.SceneData.SkyboxMaterial->GetShader();
        // s_Data.SceneInfo.EnvironmentIrradianceMap->Bind(0);
        Renderer::SubmitFullscreenQuad(s_Data.SceneData.SkyboxMaterial);

        // Bind shadow depth textures
        if (s_Data.ActiveScene && s_Data.ActiveScene->IsShadowEnabled())
        {
            for (int i = 0; i < 4; i++)
                s_Data.ShadowFBOs[i]->BindDepthTexture(PSL::PRISM_SHADOW_MAP0 + i);
        }
        // Render entities
        for (auto& dc : s_Data.DrawList)
        {
            auto baseMaterial = dc.Mesh->GetMaterial();
            // Environment
            baseMaterial->Set("u_EnvRadianceTex", s_Data.SceneData.SceneEnvironment.RadianceMap);
            baseMaterial->Set("u_EnvIrradianceTex", s_Data.SceneData.SceneEnvironment.IrradianceMap);
            baseMaterial->Set("u_BRDFLUTTexture", s_Data.BRDFLUT);

            auto overrideMaterial = dc.Material ? dc.Material : dc.Mesh->GetOverrideMaterial();
            Renderer::SubmitMesh(dc.Mesh, dc.Transform, overrideMaterial);
        }
        // 被选择实体描边
        if (outline)
        {
            Renderer::Submit([]()
                {
                    glStencilFunc(GL_ALWAYS, 1, 0xff);
                    glStencilMask(0xff);
                });
        }
        for (auto& dc : s_Data.SelectedMeshDrawList)
        {
            auto baseMaterial = dc.Mesh->GetMaterial();
            // Environment (TODO: don't do this per mesh)
            baseMaterial->Set("u_EnvRadianceTex", s_Data.SceneData.SceneEnvironment.RadianceMap);
            baseMaterial->Set("u_EnvIrradianceTex", s_Data.SceneData.SceneEnvironment.IrradianceMap);
            baseMaterial->Set("u_BRDFLUTTexture", s_Data.BRDFLUT);
            auto overrideMaterial = nullptr; // dc.Material;
            Renderer::SubmitMesh(dc.Mesh, dc.Transform, overrideMaterial);
        }

        if (outline)
        {
            Renderer::Submit([]()
                {
                    glStencilFunc(GL_NOTEQUAL, 1, 0xff);
                    glStencilMask(0);

                    glLineWidth(10);
                    glEnable(GL_LINE_SMOOTH);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    //glDisable(GL_DEPTH_TEST);
                });

            // Draw outline here
            for (auto& dc : s_Data.SelectedMeshDrawList)
            {
                Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data.OutlineMaterial);
            }

            Renderer::Submit([]()
                {
                    glPointSize(10);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                });
            for (auto& dc : s_Data.SelectedMeshDrawList)
            {
                Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data.OutlineMaterial);
            }

            Renderer::Submit([]()
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glStencilMask(0xff);
                    glStencilFunc(GL_ALWAYS, 1, 0xff);
                    //glEnable(GL_DEPTH_TEST);
                });
        }

        if (collider)
        {
            Renderer::Submit([]()
                {
                    glStencilFunc(GL_ALWAYS, 1, 0xff);
                    glStencilMask(0);

                    glLineWidth(1);
                    glEnable(GL_LINE_SMOOTH);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glDisable(GL_DEPTH_TEST);
                });

            for (auto& dc : s_Data.ColliderDrawList)
            {
                if (dc.Mesh)
                    Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data.ColliderMaterial);
            }

            Renderer::Submit([]()
                {
                    glPointSize(1);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                });

            for (auto& dc : s_Data.ColliderDrawList)
            {
                if (dc.Mesh)
                    Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data.ColliderMaterial);
            }

            Renderer::Submit([]()
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glStencilMask(0xff);
                    glStencilFunc(GL_ALWAYS, 1, 0xff);
                    glEnable(GL_DEPTH_TEST);
                });
        }
        // Grid
        if (GetOptions().ShowGrid)
        {
            Renderer::SubmitQuad(s_Data.GridMaterial, glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f)));
        }
        if (GetOptions().ShowBoundingBoxes)
        {
            auto viewProjection = s_Data.SceneData.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneData.SceneCamera.ViewMatrix;
            Renderer2D::BeginScene(viewProjection);
            for (auto& dc : s_Data.DrawList)
                Renderer::DrawAABB(dc.Mesh, dc.Transform);
            Renderer2D::EndScene();
        }
        Renderer::EndRenderPass();
    }

    void SceneRenderer::CompositePass()
    {
#if 1
        PR_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(s_Data.CompositePass);
        s_Data.CompositeMaterial->Set("u_Exposure", s_Data.SceneData.SceneCamera.Camera.GetExposure());
        s_Data.CompositeMaterial->Set("u_TextureSamples", (int)s_Data.GeoPass->GetSpecification().TargetFramebuffer->GetSpecification().Samples);
        s_Data.CompositeMaterial->Bind();
        s_Data.GeoPass->GetSpecification().TargetFramebuffer->BindTexture(PSL::PRISM_BINDING_TEXTURE);
        Renderer::SubmitFullscreenQuad(nullptr);
        Renderer::EndRenderPass();
#endif
    }

    void SceneRenderer::FlushDrawList()
    {
        if (s_Data.ActiveScene && s_Data.ActiveScene->IsShadowEnabled())
            ShadowPass();

        GeometryPass();
        CompositePass();

        s_Data.DrawList.clear();
        s_Data.SelectedMeshDrawList.clear();
        s_Data.ColliderDrawList.clear();
        s_Data.SceneData = {};
    }

    Ref<Texture2D> SceneRenderer::GetFinalColorBuffer()
    {
        // return s_Data.CompositePass->GetSpecification().TargetFramebuffer;
        PR_CORE_ASSERT(false, "Not implemented");
        return nullptr;
    }
    Ref<RenderPass> SceneRenderer::GetFinalRenderPass()
    {
        return s_Data.CompositePass;
    }

    uint32_t SceneRenderer::GetFinalColorBufferRendererID()
    {
        return s_Data.CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID();
    }
    SceneRendererOptions& SceneRenderer::GetOptions()
    {
        return s_Data.Options;
    }

    static void UpdateShadowData()
    {
        auto& camera = s_Data.SceneData.SceneCamera;
        auto& light = s_Data.ActiveScene->GetLight();
        auto& sceneUniforms = s_Data.SceneData.SceneUniforms;
        uint32_t cascadeCount = glm::clamp(s_Data.ActiveScene->GetCascadeCount(), 1u, 4u);

        // 提取相机参数
        auto& proj = camera.Camera.GetProjectionMatrix();
        float f = proj[1][1];
        float fov = 2.0f * atan(1.0f / f);
        float aspect = proj[1][1] / proj[0][0];
        float nearClip = proj[3][2] / (proj[2][2] - 1.0f);
        float farClip = proj[3][2] / (proj[2][2] + 1.0f);

        // PSSM 级联分割
        float splits[4] = {};
        float splitLambda = 0.95f;
        for (uint32_t i = 0; i < cascadeCount; i++)
        {
            float fraction = (float)(i + 1) / (float)cascadeCount;
            float logSplit = nearClip * pow(farClip / nearClip, fraction);
            float uniSplit = nearClip + (farClip - nearClip) * fraction;
            splits[i] = logSplit * splitLambda + uniSplit * (1.0f - splitLambda);
        }

        sceneUniforms.CascadeSplits = glm::vec4(splits[0], splits[1], splits[2], splits[3]);
        sceneUniforms.ShadowParams = glm::vec4(
            s_Data.ActiveScene->GetShadowBias(),
            s_Data.ActiveScene->GetShadowNormalBias(),
            (float)cascadeCount, 0.0f
        );

        glm::mat4 invView = glm::inverse(camera.ViewMatrix);
        glm::vec3 lightDir = glm::normalize(light.Direction);

        glm::vec3 up = glm::abs(lightDir.y) < 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::mat4 lightView = glm::lookAt(-lightDir, glm::vec3(0.0f), up);

        float tanHalfFov = tanf(fov * 0.5f);

        float prevSplit = nearClip;
        for (uint32_t cascade = 0; cascade < cascadeCount; cascade++)
        {
            float nextSplit = splits[cascade];


            float k = sqrtf(1.0f + aspect * aspect) * tanHalfFov;
            float centerZ = 0.0f;
            float radius = 0.0f;

            // 外接球公式到包围盒
            if (k * k >= (nextSplit - prevSplit) / (nextSplit + prevSplit)) {
                centerZ = nextSplit;
                radius = nextSplit * k;
            }
            else {
                centerZ = 0.5f * (prevSplit + nextSplit) * (1.0f + k * k);
                radius = 0.5f * sqrtf(powf(nextSplit - prevSplit, 2.0f) + 2.0f * (nextSplit * nextSplit + prevSplit * prevSplit) * k * k + powf(nextSplit + prevSplit, 2.0f) * powf(k, 4.0f));
            }

            // 观察空间到世界空间
            glm::vec3 sphereCenterWorld = glm::vec3(invView * glm::vec4(0.0f, 0.0f, -centerZ, 1.0f));

            // 光照空间
            glm::vec3 sphereCenterLight = glm::vec3(lightView * glm::vec4(sphereCenterWorld, 1.0f));

            // 外接球对称
            float minX = sphereCenterLight.x - radius;
            float maxX = sphereCenterLight.x + radius;
            float minY = sphereCenterLight.y - radius;
            float maxY = sphereCenterLight.y + radius;

            float minZ = sphereCenterLight.z - radius - 200.0f;
            float maxZ = sphereCenterLight.z + radius + 200.0f;

            // 纹素对齐
            float worldTexelSize = (2.0f * radius) / (float)SHADOW_MAP_SIZE;

            minX = floorf(minX / worldTexelSize) * worldTexelSize;
            minY = floorf(minY / worldTexelSize) * worldTexelSize;

            maxX = minX + (2.0f * radius);
            maxY = minY + (2.0f * radius);

            // 最终正交投影矩阵
            glm::mat4 lightOrtho = glm::ortho(minX, maxX, minY, maxY, -maxZ, -minZ);

            sceneUniforms.ShadowMatrices[cascade] = lightOrtho * lightView;

            prevSplit = nextSplit;
        }
    }

    static void UpdateGlobalsUBO()
    {
        auto& camera = s_Data.SceneData.SceneCamera;
        auto& sceneUniforms = s_Data.SceneData.SceneUniforms;
        //const auto& framebufferSpec = s_Data.GeoPass->GetSpecification().TargetFramebuffer->GetSpecification();
        glm::vec3 cameraPosition = glm::inverse(s_Data.SceneData.SceneCamera.ViewMatrix)[3];
        auto viewProjection = s_Data.SceneData.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneData.SceneCamera.ViewMatrix;
        //sceneUniforms.AspectRatio = (float)framebufferSpec.Width / (float)framebufferSpec.Height;
        sceneUniforms.CameraPosition = cameraPosition;
        sceneUniforms.DeltaTime = Prism::Time::GetDeltaTime();
        sceneUniforms.Projection = camera.Camera.GetProjectionMatrix();
        sceneUniforms.View = camera.ViewMatrix;
        sceneUniforms.ViewProjection = viewProjection;
        sceneUniforms.InverseViewProjection = glm::inverse(sceneUniforms.ViewProjection);
        float time = Prism::Time::GetTime();
        sceneUniforms.Time = glm::vec4(time * 0.2f, time, time * 2, time * 3);
        sceneUniforms.Lights[0] = s_Data.SceneData.ActiveLight;
        GlobalUniforms::UpdateGlobalUniform(sceneUniforms);
    }
}
