#include "prpch.h"
#include "RenderSystem.h"
#include "Physics3DSystem.h"

#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Editor/EditorCamera.h"

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
            auto& transform = camEntity.GetComponent<TransformComponent>();
            m_PendingFrameData.Camera.Projection = camComp.Camera;
            m_PendingFrameData.Camera.ViewMatrix = glm::inverse(transform.GetTransform());
        }

        m_Config.SkyboxMaterial->SetFloat("u_TextureLod", m_Config.SkyboxLod);

        if (m_Config.SceneEnvironment.RadianceMap)
            m_Config.SkyboxMaterial->SetTexture("u_Texture", m_Config.SceneEnvironment.RadianceMap);

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
        auto group = m_Scene->GetRegistry().group<MeshRendererComponent, TransformComponent>();
        for (auto& entity : group)
        {
            auto [renderer, transform] = group.get<MeshRendererComponent, TransformComponent>(entity);

            if (!renderer.Mesh) continue;

            renderer.Mesh->OnUpdate(ts);

            glm::mat4 worldTransform = transform.GetTransform();
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
