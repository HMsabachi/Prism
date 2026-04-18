#pragma once
#include <Prism.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
//#include "Prism/Core/EntryPoint.h"
#include "glm/gtc/type_ptr.inl"


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

		enum class PropertyFlag
		{
			None = 0, ColorProperty = 1
		};

		virtual void OnImGuiRender() override;

		virtual void OnEvent(Event& event) override;

		bool OnKeyPressedEvent(KeyPressedEvent& e);

		bool Property(const std::string& name, bool& value);
		void Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		void Property(const std::string& name, glm::vec2& value, PropertyFlag flags);
		void Property(const std::string& name, glm::vec2& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		void Property(const std::string& name, glm::vec3& value, PropertyFlag flags);
		void Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		void Property(const std::string& name, glm::vec4& value, PropertyFlag flags);
		void Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);

		void ShowBoundingBoxes(bool show, bool onTop = false);
	private:
		std::vector<Ref<MaterialInstance>> m_MetalSphereMaterialInstances;
		std::vector<Ref<MaterialInstance>> m_DielectricSphereMaterialInstances;

		float m_GridScale = 16.025f, m_GridSize = 0.025f;
	private:
		Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;

		Ref<Scene> m_Scene;
		Ref<Scene> m_SphereScene;
		Ref<Scene> m_ActiveScene;

		Entity* m_MeshEntity = nullptr;

		Ref<Shader> m_BrushShader;
		Ref<Mesh> m_PlaneMesh;
		Ref<Material> m_SphereBaseMaterial;

		Ref<Material> m_MeshMaterial;

		struct AlbedoInput
		{
			glm::vec3 Color = { 0.972f, 0.96f, 0.915f }; // Silver, from https://docs.unrealengine.com/en-us/Engine/Rendering/Materials/PhysicallyBased
			Ref<Texture2D> TextureMap;
			bool SRGB = true;
			bool UseTexture = false;
		};
		AlbedoInput m_AlbedoInput;

		struct NormalInput
		{
			Ref<Texture2D> TextureMap;
			bool UseTexture = false;
		};
		NormalInput m_NormalInput;

		struct MetalnessInput
		{
			float Value = 1.0f;
			Ref<Texture2D> TextureMap;
			bool UseTexture = false;
		};
		MetalnessInput m_MetalnessInput;

		struct RoughnessInput
		{
			float Value = 0.2f;
			Ref<Texture2D> TextureMap;
			bool UseTexture = false;
		};
		RoughnessInput m_RoughnessInput;

		struct Light
		{
			glm::vec3 Direction;
			glm::vec3 Radiance;
		};
		Light m_Light;
		float m_LightMultiplier = 0.3f;

		bool m_RadiancePrefilter = false;

		float m_EnvMapRotation = 0.0f;

		enum class SceneType : uint32_t
		{
			Spheres = 0, Model = 1
		};
		SceneType m_SceneType;

		// Editor resources
		Ref<Texture2D> m_CheckerboardTex;

		int m_GizmoType = -1; //  no gizmo

		bool m_AllowViewportCameraEvents = true;
		bool m_DrawOnTopBoundingBoxes = false;

		bool m_UIShowBoundingBoxes = false;
		bool m_UIShowBoundingBoxesOnTop = false;
	};
}