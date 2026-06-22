#include "prpch.h"
#include "RenderSystem.h"
#include "Physics3DSystem.h"

#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Editor/EditorCamera.h"
#include "Prism/ImGui/ImGui.h"
#include "Prism/Core/LanguageManager.h"

namespace Prism
{
    RenderSystem::RenderSystem(Scene* scene)
        : m_Scene(scene)
    {
    }
    RenderSystem::~RenderSystem()
    {
    }
#pragma region 生命周期
    void RenderSystem::OnCreate()
    {
        m_Pipeline = std::make_unique<RenderPipeline>();
        m_Pipeline->Initialize(1280, 720);

        auto skyboxShader = Renderer::GetShaderLibrary()->Get("Custom/Skybox");
        m_Config.SkyboxMaterial = Material::Create(skyboxShader);
    }

    void RenderSystem::OnDestroy()
    {
        if (m_Pipeline)
            m_Pipeline->Shutdown();
        m_Pipeline.reset();
    }
    void RenderSystem::OnPostLateUpdate(float dt)
    {
        Render();
    }


	void RenderSystem::OnImGuiRender()
	{
        ImGui::Begin("Render System");
        if (UI::BeginTreeNode(TR("Shadows")))
        {
            UI::BeginPropertyGrid();
            UI::Property("Enable Shadows", m_Config.ShadowsEnabled);
            UI::Property("Shadow Bias", m_Config.ShadowBias, 0.0001f, 0.0f, 0.1f);
            UI::Property("Normal Bias", m_Config.ShadowNormalBias, 0.001f, 0.0f, 1.0f);
            UI::Property("Cascade Count", m_Config.CascadeCount, 1, 4);
            UI::Property("Max Shadow Distance", m_Config.MaxShadowDistance, 1.0f, 1000.0f, UI::PropertyFlag::SliderProperty);
            UI::EndPropertyGrid();
            UI::EndTreeNode();
        }
        if (UI::BeginTreeNode(TR("Shadow Map"), false))
        {
            static int cascadeIndex = 0;
            UI::BeginPropertyGrid();
            UI::PropertySlider("Cascade Index", cascadeIndex, 0, 3);
            UI::EndPropertyGrid();
            const auto& rendererID = m_Pipeline->GetShadowPass(cascadeIndex)->GetSpecification().TargetFramebuffer->GetDepthAttachmentRendererID();
            float size = ImGui::GetContentRegionAvail().x;
            ImGui::Image((void*)(uint64_t)rendererID, { size, size }, { 0, 1 }, { 1, 0 });
            UI::EndTreeNode();
        }
        ImGui::End();
	}

	void RenderSystem::Render()
    {
        m_PendingFrameData.DrawList.clear();
        m_PendingFrameData.SelectedDrawList.clear();

        if (m_HasEditorCamera)
        {
            m_PendingFrameData.Camera = m_EditorCamera;
        }
        else
        {
            Entity camEntity = m_Scene->GetMainCameraEntity();
            if (!camEntity) return;
            auto& camComp = camEntity.GetComponent<CameraComponent>();
            camComp.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
            auto& transform = camEntity.Transformation();
            m_PendingFrameData.Camera.Projection = camComp.Camera;
            m_PendingFrameData.Camera.ViewMatrix = glm::inverse(transform.GetMatrix());
        }

        // Process lights
        {
            m_Config.LightEnvironment = LightEnvironment();
            auto lights = m_Scene->GetRegistry().group<TransformComponent, DirectionalLightComponent>();
            uint32_t directionalLightIndex = 0;
            for (auto entity : lights)
            {
                auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>(entity);
                Transform& lightTransform = transformComponent.Transformation;
                glm::vec3 direction = -glm::normalize(glm::mat3(lightTransform.GetMatrix()) * glm::vec3(1.0f));
                m_Config.LightEnvironment.DirectionalLights[directionalLightIndex++] =
                {
                    direction,
                    lightComponent.Radiance,
                    lightComponent.Intensity,
                    lightComponent.CastShadows,
                    lightComponent.SoftShadows,
                    lightComponent.LightSize
                };
            }
        }

        // TODO: only one sky light at the moment!
        {
            auto lights = m_Scene->GetRegistry().view<SkyLightComponent>();
            for (auto entity : lights)
            {
                auto& skyLightComponent = lights.get<SkyLightComponent>(entity);
                m_Config.SceneEnvironment = skyLightComponent.SceneEnvironment;
                m_Config.SceneEnvironmentIntensity = skyLightComponent.Intensity;
                if (m_Config.SceneEnvironment.RadianceMap)
                    m_Config.SkyboxMaterial->SetTexture("u_Texture", m_Config.SceneEnvironment.RadianceMap);
            }
        }

        m_Config.SkyboxMaterial->SetFloat("u_TextureLod", m_Config.SkyboxLod);

        CollectMeshRenderers(m_PendingFrameData);
        CollectDebugDraws(m_PendingFrameData);

        if (m_Pipeline)
            m_Pipeline->Execute(m_Config, m_PendingFrameData);

        m_PendingFrameData.DebugDrawList.clear();
    }
#pragma endregion

    void RenderSystem::SetEditorCamera(const EditorCamera& camera)
    {
        m_EditorCamera.Projection = camera;
        m_EditorCamera.ViewMatrix = camera.GetViewMatrix();
        m_HasEditorCamera = true;
    }

    void RenderSystem::SetViewportSize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || width == m_ViewportWidth && height == m_ViewportHeight)
            return;
        m_ViewportWidth = width;
        m_ViewportHeight = height;
        if (m_Pipeline)
            m_Pipeline->Resize(width, height);
    }

    void RenderSystem::SubmitDebugMesh(Ref<Mesh> mesh, const glm::mat4& transform,
                                        Ref<Material> material)
    {
        for (uint32_t i = 0; i < mesh->GetSubmeshes().size(); i++)
            m_PendingFrameData.DebugDrawList.push_back({ mesh, i, material, transform });
    }

    Ref<RenderPass> RenderSystem::GetFinalRenderPass()
    {
        return m_Pipeline ? m_Pipeline->GetFinalRenderPass() : nullptr;
    }

    uint32_t RenderSystem::GetFinalColorBufferID()
    {
        return m_Pipeline ? m_Pipeline->GetFinalRenderPass()
            ->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID() : 0;
    }

    RenderPipelineOptions& RenderSystem::GetOptions()
    {
        return m_Pipeline->GetOptions();
    }

    std::pair<Ref<TextureCube>, Ref<TextureCube>>
    RenderSystem::CreateEnvironmentMap(const std::string& filepath)
    {
        return RenderPipeline::CreateEnvironmentMap(filepath);
    }


    void RenderSystem::CollectMeshRenderers(FrameData& data)
    {
        float ts = Time::GetDeltaTime();
        glm::vec3 camPos = glm::inverse(data.Camera.ViewMatrix)[3];
        auto view = m_Scene->GetRegistry().view<MeshRendererComponent>();

        for (auto& entity : view)
        {
            auto& renderer = view.get<MeshRendererComponent>(entity);
            auto& transform = m_Scene->GetRegistry().get<TransformComponent>(entity);

            if (!renderer.Mesh) continue;

            renderer.Mesh->OnUpdate(ts);

            glm::mat4 worldTransform = transform.Transformation.GetMatrix();
            bool isSelected = (m_Scene->GetSelectedEntity() == entity);

            for (uint32_t i = 0; i < renderer.Mesh->GetSubmeshes().size(); i++)
            {
                auto& submesh = renderer.Mesh->GetSubmeshes()[i];
                Ref<Material> mat = (i < renderer.Materials.size()) ? renderer.Materials[i] : nullptr;

                DrawCommand cmd;
                cmd.Mesh = renderer.Mesh;
                cmd.SubmeshIndex = i;
                cmd.Material = mat;
                cmd.Transform = worldTransform * submesh.Transform;

                uint64_t program = mat ? (uint64_t)mat->GetProgram().Raw() : 0;
                uint64_t material = (uint64_t)mat.Raw();
                uint64_t mesh = (uint64_t)renderer.Mesh.Raw();
                float dist = glm::distance(glm::vec3(cmd.Transform[3]), camPos);
                uint64_t distQ = (uint64_t)(dist * 10.0f) & 0xFFFF;
                cmd.SortKey = ((program & 0xFFFF) << 48) | ((material & 0xFFFF) << 32) | ((mesh & 0xFFFF) << 16) | distQ;

                if (isSelected)
                    data.SelectedDrawList.push_back(cmd);
                else
                    data.DrawList.push_back(cmd);
            }
        }
    }

    void RenderSystem::CollectDebugDraws(FrameData& data)
    {
        auto* physics = m_Scene->GetSystem<Physics3DSystem>();
        if (physics)
            physics->SubmitColliderMeshes();
    }
}
