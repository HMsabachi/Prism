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

		void Property(const std::string& name, bool& value);
		void Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		void Property(const std::string& name, glm::vec3& value, PropertyFlag flags);
		void Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		void Property(const std::string& name, glm::vec4& value, PropertyFlag flags);
		void Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);

	private:
		glm::vec3 m_ModelPosition = { 0.f, 0.0f, 0.0f };
		float m_ModelRotation = 0.0f;
		Ref<MaterialInstance> m_MeshMaterial;
		Ref<MaterialInstance> m_GridMaterial;
		std::vector<Ref<MaterialInstance>> m_MetalSphereMaterialInstances;
		std::vector<Ref<MaterialInstance>> m_DielectricSphereMaterialInstances;

		float m_GridScale = 16.025f, m_GridSize = 0.025f;
		float m_MeshScale = 1.0f;
	private:
		Ref<PrismShader> m_QuadShader;
		Ref<PrismShader> m_HDRShader;
		Ref<PrismShader> m_GridShader;
		Ref<ComputeShader> m_ComShader;
		Ref<Mesh> m_Mesh;
		Ref<Mesh> m_SphereMesh, m_PlaneMesh;
		Ref<Texture2D> m_BRDFLUT;
		Ref<RenderPass> m_GeoPass, m_CompositePass;

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
			float Value = 0.5f;
			Ref<Texture2D> TextureMap;
			bool UseTexture = false;
		};
		RoughnessInput m_RoughnessInput;

		Ref<VertexArray> m_FullscreenQuadVertexArray;
		Ref<TextureCube> m_EnvironmentCubeMap, m_EnvironmentIrradiance;

		Camera m_Camera;

		struct Light
		{
			glm::vec3 Direction;
			glm::vec3 Radiance;
		};
		Light m_Light;
		float m_LightMultiplier = 0.3f;

		// PBR params
		float m_Exposure = 1.0f;

		bool m_RadiancePrefilter = false;

		float m_EnvMapRotation = 0.0f;

		enum class Scene : uint32_t
		{
			Spheres = 0, Model = 1
		};
		Scene m_Scene;

		// Editor resources
		Ref<Texture2D> m_CheckerboardTex;

		int m_GizmoType = -1; // -1 = no gizmo
		glm::mat4 m_Transform;


		Ref<Texture2D> envEquirect;
	};
}