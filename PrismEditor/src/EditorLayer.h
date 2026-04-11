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

		void Property(const std::string& name, bool& value);

		void Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);

		void Property(const std::string& name, glm::vec3& value, PropertyFlag flags);

		void Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);

		void Property(const std::string& name, glm::vec4& value, PropertyFlag flags);

		void Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);

		virtual void OnImGuiRender() override;

		virtual void OnEvent(Prism::Event& event) override;

	private:
		void UpdateGlobalsUBO();
	private:
		glm::vec3 m_ModelPosition = { 0.f, 0.0f, 0.0f };
		float m_ModelRotation = 0.0f;
		Prism::Ref<Prism::MaterialInstance> m_MeshMaterial;
		Prism::Ref<Prism::MaterialInstance> m_GridMaterial;
		std::vector<Prism::Ref<Prism::MaterialInstance>> m_MetalSphereMaterialInstances;
		std::vector<Prism::Ref<Prism::MaterialInstance>> m_DielectricSphereMaterialInstances;

		float m_GridScale = 16.025f, m_GridSize = 0.025f;
		float m_MeshScale = 1.0f;
	private:
		Prism::PrismGlobalsUBO m_GlobalsUBO;
		Prism::Ref<Prism::PrismShader> m_QuadShader;
		Prism::Ref<Prism::PrismShader> m_HDRShader;
		Prism::Ref<Prism::PrismShader> m_GridShader;
		Prism::Ref<Prism::Mesh> m_Mesh;
		Prism::Ref<Prism::Mesh> m_SphereMesh, m_PlaneMesh;
		Prism::Ref<Prism::Texture2D> m_BRDFLUT;

		struct AlbedoInput
		{
			glm::vec3 Color = { 0.972f, 0.96f, 0.915f }; // Silver, from https://docs.unrealengine.com/en-us/Engine/Rendering/Materials/PhysicallyBased
			Prism::Ref<Prism::Texture2D> TextureMap;
			bool SRGB = true;
			bool UseTexture = false;
		};
		AlbedoInput m_AlbedoInput;

		struct NormalInput
		{
			Prism::Ref<Prism::Texture2D> TextureMap;
			bool UseTexture = false;
		};
		NormalInput m_NormalInput;

		struct MetalnessInput
		{
			float Value = 1.0f;
			Prism::Ref<Prism::Texture2D> TextureMap;
			bool UseTexture = false;
		};
		MetalnessInput m_MetalnessInput;

		struct RoughnessInput
		{
			float Value = 0.5f;
			Prism::Ref<Prism::Texture2D> TextureMap;
			bool UseTexture = false;
		};
		RoughnessInput m_RoughnessInput;

		Prism::Ref<Prism::Framebuffer> m_Framebuffer, m_FinalPresentBuffer;

		Prism::Ref<Prism::VertexBuffer> m_VertexBuffer;
		Prism::Ref<Prism::IndexBuffer> m_IndexBuffer;
		Prism::Ref<Prism::TextureCube> m_EnvironmentCubeMap, m_EnvironmentIrradiance;

		Prism::Camera m_Camera;

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
		std::unique_ptr<Prism::Texture2D> m_CheckerboardTex;
	};
}