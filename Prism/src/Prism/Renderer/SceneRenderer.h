#pragma once

#include "Prism/Scene/Scene.h"

#include "Prism/Renderer/Mesh.h"
#include "Prism/Scene/Components.h"
#include "RenderPass.h"
#include "Prism/Renderer/Camera/Camera.h"

namespace Prism
{
    class Camera;
}

namespace Prism 
{
    struct SceneRendererOptions
    {
        bool ShowGrid = true;
        bool ShowBoundingBoxes = false;
    };
    struct SceneRendererCamera
    {
        Prism::Camera Camera;
        glm::mat4 ViewMatrix;
    };

    class PRISM_API SceneRenderer
    {
    public:
        static void Init();

        static void SetViewportSize(uint32_t width, uint32_t height);

        static void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
        static void EndScene();

        static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f), Ref<MaterialInstance> overrideMaterial = nullptr);
        static void SubmitSelectedMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f));
        static void SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& parentTransform = glm::mat4(1.0F));
        static void SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& parentTransform = glm::mat4(1.0F));
        static void SubmitColliderMesh(const CapsuleColliderComponent& component, const glm::mat4& parentTransform = glm::mat4(1.0F));
        static void SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& parentTransform = glm::mat4(1.0F));

        static std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath);

        static Ref<RenderPass> GetFinalRenderPass();

        static Ref<Texture2D> GetFinalColorBuffer();

        static uint32_t GetFinalColorBufferRendererID();

        static SceneRendererOptions& GetOptions();
    private:
        static void FlushDrawList();
        static void ShadowPass();
        static void GeometryPass();
        static void CompositePass();
    };

}