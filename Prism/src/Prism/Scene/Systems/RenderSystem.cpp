#include "prpch.h"
#include "RenderSystem.h"
#include "InterpolationSystem.h"
#include "Physics3DSystem.h"

#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/RenderPass.h"
#include "Prism/Renderer/SceneRenderer.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Buffer/Framebuffer.h"
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
        auto skyboxShader = AssetManager::GetShaderLibrary()->Get("Custom/Skybox");
        m_Config.SkyboxMaterial = Material::Create(skyboxShader);
    }

    void RenderSystem::OnDestroy()
    {
    }
    void RenderSystem::OnRender(float dt)
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
        if (UI::BeginTreeNode(TR("Bloom")))
        {
            UI::BeginPropertyGrid();
            UI::Property("Enable Bloom", m_Config.EnableBloom);
            UI::Property("Bloom Threshold", m_Config.BloomThreshold, 0.1f, 0.0f, 10.0f);

            UI::EndPropertyGrid();
            UI::EndTreeNode();
        }
        ImGui::End();
    }

    void RenderSystem::Render()
    {
        PR_PROFILE_FUNCTION();
        m_PendingSnapshot.DrawList.clear();
        m_PendingSnapshot.SelectedDrawList.clear();

        if (m_HasEditorCamera)
        {
            m_PendingSnapshot.Camera = m_EditorCamera;
        }
        else
        {
            Entity camEntity = m_Scene->GetMainCameraEntity();
            if (!camEntity) return;
            auto& camComp = camEntity.GetComponent<CameraComponent>();
            camComp.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
            m_PendingSnapshot.Camera.Projection = camComp.Camera;
            auto* interpSys = m_Scene->GetSystem<InterpolationSystem>();
            glm::mat4 worldMatrix = (m_Scene->IsPlaying() && interpSys)
                ? interpSys->GetInterpolatedWorldMatrix(camEntity)
                : m_Scene->GetTransformRelativeToParent(camEntity);
            glm::vec3 right   = glm::normalize(glm::vec3(worldMatrix[0]));
            glm::vec3 up      = glm::normalize(glm::vec3(worldMatrix[1]));
            glm::vec3 forward = glm::normalize(glm::vec3(worldMatrix[2]));
            glm::vec3 newRight = glm::normalize(glm::cross(up, forward));
            glm::vec3 newUp    = glm::normalize(glm::cross(forward, newRight));
            glm::vec3 pos = glm::vec3(worldMatrix[3]);
            glm::mat4 rigidWorld(1.0f);
            rigidWorld[0] = glm::vec4(newRight, 0.0f);
            rigidWorld[1] = glm::vec4(newUp, 0.0f);
            rigidWorld[2] = glm::vec4(forward, 0.0f);
            rigidWorld[3] = glm::vec4(pos, 1.0f);
            m_PendingSnapshot.Camera.ViewMatrix = glm::inverse(rigidWorld);
        }

        // Process lights
        {
            m_Config.LightEnvironment = LightEnvironment();
            auto lights = m_Scene->GetRegistry().group<TransformComponent, DirectionalLightComponent>();
            uint32_t directionalLightIndex = 0;
            for (auto entity : lights)
            {
                auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>(entity);
                glm::mat4 worldMatrix = m_Scene->GetTransformRelativeToParent(Entity(entity, m_Scene));
                glm::vec3 direction = -glm::normalize(glm::mat3(worldMatrix) * glm::vec3(1.0f));
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
                m_Config.SkyboxLod = skyLightComponent.SkyboxLod;
                if (m_Config.SceneEnvironment && m_Config.SceneEnvironment->RadianceMap)
                    m_Config.SkyboxMaterial->SetTexture("u_Texture", m_Config.SceneEnvironment->RadianceMap);
            }
        }

        m_Config.SkyboxMaterial->SetFloat("u_TextureLod", m_Config.SkyboxLod);

        CollectMeshRenderers(m_PendingSnapshot);
        CollectDebugDraws(m_PendingSnapshot);

        m_PendingSnapshot.Config = m_Config;

        SceneRenderer::Get().Execute(m_PendingSnapshot);

        m_PendingSnapshot.DebugDrawList.clear();
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
    }

    void RenderSystem::SubmitDebugMesh(Ref<Mesh> mesh, const glm::mat4& transform,
                                        Ref<Material> material)
    {
        for (uint32_t i = 0; i < mesh->GetSubmeshes().size(); i++)
            m_PendingSnapshot.DebugDrawList.push_back({ mesh, i, material, transform });
    }

    void RenderSystem::CollectMeshRenderers(FrameSnapshot& snapshot)
    {
        float ts = Time::GetDeltaTime();
        glm::vec3 camPos = glm::inverse(snapshot.Camera.ViewMatrix)[3];
        auto view = m_Scene->GetRegistry().view<MeshRendererComponent>();

        auto* interpSys = m_Scene->GetSystem<InterpolationSystem>();

        for (auto& entity : view)
        {
            auto& renderer = view.get<MeshRendererComponent>(entity);

            if (!renderer.Mesh) continue;

            renderer.Mesh->OnUpdate(ts);

            Entity e = { entity, m_Scene };

            glm::mat4 worldTransform;
            if (m_Scene->IsPlaying() && interpSys)
                worldTransform = interpSys->GetInterpolatedWorldMatrix(e);
            else
                worldTransform = m_Scene->GetTransformRelativeToParent(e);
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

                //uint64_t program = mat ? (uint64_t)mat->GetProgram().Raw() : 0;
                //uint64_t material = (uint64_t)mat.Raw();
                //uint64_t mesh = (uint64_t)renderer.Mesh.Raw();
                //float dist = glm::distance(glm::vec3(cmd.Transform[3]), camPos);
                //uint64_t distQ = (uint64_t)(dist * 10.0f) & 0xFFFF;
                //cmd.SortKey = ((program & 0xFFFF) << 48) | ((material & 0xFFFF) << 32) | ((mesh & 0xFFFF) << 16) | distQ;

                if (isSelected)
                    snapshot.SelectedDrawList.push_back(cmd);
                snapshot.DrawList.push_back(cmd);
            }
        }
    }

    void RenderSystem::CollectDebugDraws(FrameSnapshot& snapshot)
    {
        auto* physics = m_Scene->GetSystem<Physics3DSystem>();
        if (physics)
            physics->SubmitColliderMeshes();
    }
}
