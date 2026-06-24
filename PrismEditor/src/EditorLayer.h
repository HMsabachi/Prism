#pragma once
#include <Prism.h>
#include "Prism/Events/KeyEvent.h"
#include "Prism/Events/MouseEvent.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "glm/gtc/type_ptr.hpp"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>

namespace Prism
{
    class EditorLayer : public Prism::Layer
    {
    public:
        EditorLayer();


        virtual ~EditorLayer();

        virtual void OnAttach() override;

        virtual void OnDetach() override;

        virtual void OnUpdate() override;

        virtual void OnImGuiRender() override;

        virtual void OnEvent(Event& event) override;
        bool OnKeyPressedEvent(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void ShowBoundingBoxes(bool show, bool onTop = false);
        void UpdateWindowTitle(const std::string& sceneName);
        void SelectEntity(Entity entity);
        void DrawMaterialProperty(const PrismShaderCompiler::AST::ShaderUniform& uni, Material& material);

        void OpenScene();
        void SaveScene();
        void SaveSceneAs();
    private:
        std::pair<float, float> GetMouseViewportSpace();
        std::pair<glm::vec3, glm::vec3> CastRay(float mx, float my);

        struct SelectedSubmesh
        {
            Prism::Entity Entity;
            Submesh* Mesh = nullptr;
            float Distance = 0.0f;
        };

        void OnSelected(const SelectedSubmesh& selectionContext);
        void OnEntityDeleted(Entity e);
        Ray CastMouseRay();

        void OnScenePlay();
        void OnSceneStop();

        float GetSnapValue();
    private:
        std::vector<Ref<Material>> m_MetalSphereMaterialInstances;
        std::vector<Ref<Material>> m_DielectricSphereMaterialInstances;
    private:
        Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;

        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_RuntimeScene, m_EditorScene;

        EditorCamera m_EditorCamera;

        Ref<Material> m_SphereBaseMaterial;

        Ref<Material> m_MeshMaterial;

        bool m_RadiancePrefilter = false;

        float m_EnvMapRotation = 0.0f;

        enum class SceneType : uint32_t
        {
            Spheres = 0, Model = 1
        };
        SceneType m_SceneType;

        // Editor resources
        Ref<Texture2D> m_CheckerboardTex;
        Ref<Texture2D> m_PlayButtonTex;

        glm::vec2 m_ViewportBounds[2]{};
        int m_GizmoType = -1; //  no gizmo
        float m_SnapValue = 0.5f;
        float m_RotationSnapValue = 45.0f;
        bool m_AllowViewportCameraEvents = true;
        bool m_DrawOnTopBoundingBoxes = false;

        bool m_UIShowBoundingBoxes = false;
        bool m_UIShowBoundingBoxesOnTop = false;

        bool m_ViewportPanelMouseOver = false;
        bool m_ViewportPanelFocused = false;
        bool m_ReloadScriptOnPlay = false;

        bool m_ShowPhysicsSettings = false;
        std::string m_SceneFilePath;

        enum class SceneState
        {
            Edit = 0, Play = 1, Pause = 2
        };
        SceneState m_SceneState = SceneState::Edit;


        enum class SelectionMode
        {
            None = 0, Entity = 1, SubMesh = 2
        };

        SelectionMode m_SelectionMode = SelectionMode::Entity;
        std::vector<SelectedSubmesh> m_SelectionContext;
        glm::mat4* m_RelativeTransform = nullptr;
        glm::mat4* m_CurrentlySelectedTransform = nullptr;
    };
}
