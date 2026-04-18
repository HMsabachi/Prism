#include "EditorLayer.h"

#include "Prism/ImGui/ImGuizmo.h"

namespace Prism
{
	static void ImGuiShowHelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}


	EditorLayer::EditorLayer() : m_SceneType(SceneType::Model)
	{

	}

	EditorLayer::~EditorLayer()
	{

	}

	void EditorLayer::OnAttach()
	{
		#pragma region ImGui Color
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.30f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.58f, 0.58f, 0.58f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.96f, 0.00f, 0.00f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.00f, 0.00f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.64f, 0.06f, 0.00f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.12f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 1.00f, 0.12f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(1.00f, 0.00f, 0.00f, 0.69f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.85f, 0.85f, 0.85f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 0.00f, 0.00f, 0.62f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.38f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.97f, 0.00f, 0.00f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.80f, 0.00f, 0.00f, 0.86f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
		colors[ImGuiCol_TabActive] = ImVec4(1.00f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.36f, 0.36f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.60f);



		#pragma endregion

		using namespace glm;
		auto environment = Environment::Load("Assets/env/birchwood_4k.hdr");
		// Model Scene
		{
			m_Scene = CreateRef<Scene>("Model Scene");
			m_Scene->SetCamera(Camera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f)));
			m_Scene->SetEnvironment(environment);
			m_MeshEntity = m_Scene->CreateEntity();
			auto mesh = CreateRef<Mesh>("assets/models/m1911/m1911.fbx");
			//auto mesh = CreateRef<Mesh>("assets/meshes/cerberus/CerberusMaterials.fbx");
			// auto mesh = CreateRef<Mesh>("assets/models/m1911/M1911Materials.fbx");
			m_MeshEntity->SetMesh(mesh);
			m_MeshMaterial = mesh->GetMaterial();
		}

		// Sphere Scene
		{
			m_SphereScene = CreateRef<Scene>("PBR Sphere Scene");
			m_SphereScene->SetCamera(Camera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f)));
			m_SphereScene->SetEnvironment(environment);
			auto sphereMesh = CreateRef<Mesh>("assets/models/Sphere1m.fbx");
			m_SphereBaseMaterial = sphereMesh->GetMaterial();
			float x = -4.0f;
			float roughness = 0.0f;
			for (int i = 0; i < 8; i++)
			{
				auto sphereEntity = m_SphereScene->CreateEntity();

				Ref<MaterialInstance> mi = CreateRef<MaterialInstance>(m_SphereBaseMaterial);
				mi->Set("u_Metalness", 1.0f);
				mi->Set("u_Roughness", roughness);
				x += 1.1f;
				roughness += 0.15f;
				m_MetalSphereMaterialInstances.push_back(mi);

				sphereEntity->SetMesh(sphereMesh);
				sphereEntity->SetMaterial(mi);
				sphereEntity->Transform() = translate(mat4(1.0f), vec3(x, 0.0f, 0.0f));
				x = -4.0f;
				roughness = 0.0f;
				for (int i = 0; i < 8; i++)
				{
					auto sphereEntity = m_SphereScene->CreateEntity();

					Ref<MaterialInstance> mi(new MaterialInstance(m_SphereBaseMaterial));
					mi->Set("u_Metalness", 0.0f);
					mi->Set("u_Roughness", roughness);
					x += 1.1f;
					roughness += 0.15f;
					m_DielectricSphereMaterialInstances.push_back(mi);

					sphereEntity->SetMesh(sphereMesh);
					sphereEntity->SetMaterial(mi);
					sphereEntity->Transform() = translate(mat4(1.0f), vec3(x, 1.2f, 0.0f));
				}
			}
		}
		m_ActiveScene = m_Scene;
		m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>(m_ActiveScene);

		m_PlaneMesh.reset(new Mesh("Assets/Models/Plane1m.obj"));
		// Editor
		m_CheckerboardTex = Texture2D::Create("assets/editor/Checkerboard.tga");
		m_Light.Direction = { -0.5f, -0.5f, 1.0f };
		m_Light.Radiance = { 1.0f, 1.0f, 1.0f };
	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate()
	{
		// THINGS TO LOOK AT:
		// - BRDF LUT
		// - Cubemap mips and filtering
		// - Tonemapping and proper HDR pipeline
		using namespace Prism;
		using namespace glm;

		m_MeshMaterial->Set("u_AlbedoColor", m_AlbedoInput.Color);
		m_MeshMaterial->Set("u_Metalness", m_MetalnessInput.Value);
		m_MeshMaterial->Set("u_Roughness", m_RoughnessInput.Value);

		m_MeshMaterial->Set("u_LightDirection", m_Light.Direction);
		m_MeshMaterial->Set("u_LightRadiance", m_Light.Radiance);
		m_MeshMaterial->Set("u_RadiancePrefilter", m_RadiancePrefilter ? 1.0f : 0.0f);
		m_MeshMaterial->Set("u_AlbedoTexToggle", m_AlbedoInput.UseTexture ? 1.0f : 0.0f);
		m_MeshMaterial->Set("u_NormalTexToggle", m_NormalInput.UseTexture ? 1.0f : 0.0f);
		m_MeshMaterial->Set("u_MetalnessTexToggle", m_MetalnessInput.UseTexture ? 1.0f : 0.0f);
		m_MeshMaterial->Set("u_RoughnessTexToggle", m_RoughnessInput.UseTexture ? 1.0f : 0.0f);
		m_MeshMaterial->Set("u_EnvMapRotation", m_EnvMapRotation);

		if (m_AlbedoInput.TextureMap)
			m_MeshMaterial->Set("u_AlbedoTexture", m_AlbedoInput.TextureMap);
		if (m_NormalInput.TextureMap)
			m_MeshMaterial->Set("u_NormalTexture", m_NormalInput.TextureMap);
		if (m_MetalnessInput.TextureMap)
			m_MeshMaterial->Set("u_MetalnessTexture", m_MetalnessInput.TextureMap);
		if (m_RoughnessInput.TextureMap)
			m_MeshMaterial->Set("u_RoughnessTexture", m_RoughnessInput.TextureMap);

		m_SphereBaseMaterial->Set("u_AlbedoColor", m_AlbedoInput.Color);
		m_SphereBaseMaterial->Set("u_Metalness", m_MetalnessInput.Value);
		m_SphereBaseMaterial->Set("u_Roughness", m_RoughnessInput.Value);

		m_SphereBaseMaterial->Set("u_LightDirection", m_Light.Direction);
		m_SphereBaseMaterial->Set("u_LightRadiance", m_Light.Radiance);
		m_SphereBaseMaterial->Set("u_RadiancePrefilter", m_RadiancePrefilter ? 1.0f : 0.0f);
		m_SphereBaseMaterial->Set("u_AlbedoTexToggle", m_AlbedoInput.UseTexture ? 1.0f : 0.0f);
		m_SphereBaseMaterial->Set("u_NormalTexToggle", m_NormalInput.UseTexture ? 1.0f : 0.0f);
		m_SphereBaseMaterial->Set("u_MetalnessTexToggle", m_MetalnessInput.UseTexture ? 1.0f : 0.0f);
		m_SphereBaseMaterial->Set("u_RoughnessTexToggle", m_RoughnessInput.UseTexture ? 1.0f : 0.0f);
		m_SphereBaseMaterial->Set("u_EnvMapRotation", m_EnvMapRotation);

		m_ActiveScene->OnUpdate();

		//Renderer::SubmitMesh(m_PlaneMesh, glm::scale(glm::mat4(1.0f), glm::vec3(16.0f)), m_GridMaterial);
	}

	void EditorLayer::Property(const std::string& name, glm::vec4& value, float min /*= -1.0f*/, float max /*= 1.0f*/, PropertyFlag flags /*= PropertyFlag::None*/)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		if ((int)flags & (int)PropertyFlag::ColorProperty)
			ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
		else
			ImGui::SliderFloat4(id.c_str(), glm::value_ptr(value), min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}
	void EditorLayer::Property(const std::string& name, glm::vec2& value, EditorLayer::PropertyFlag flags)
	{
		Property(name, value, -1.0f, 1.0f, flags);
	}

	void EditorLayer::Property(const std::string& name, glm::vec2& value, float min, float max, EditorLayer::PropertyFlag flags)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		ImGui::SliderFloat2(id.c_str(), glm::value_ptr(value), min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	void EditorLayer::Property(const std::string& name, glm::vec4& value, PropertyFlag flags)
	{
		Property(name, value, -1.0f, 1.0f, flags);
	}

	void EditorLayer::Property(const std::string& name, glm::vec3& value, float min /*= -1.0f*/, float max /*= 1.0f*/, PropertyFlag flags /*= PropertyFlag::None*/)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		if ((int)flags & (int)PropertyFlag::ColorProperty)
			ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
		else
			ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	void EditorLayer::Property(const std::string& name, glm::vec3& value, PropertyFlag flags)
	{
		Property(name, value, -1.0f, 1.0f, flags);
	}

	void EditorLayer::Property(const std::string& name, float& value, float min /*= -1.0f*/, float max /*= 1.0f*/, PropertyFlag flags /*= PropertyFlag::None*/)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		ImGui::SliderFloat(id.c_str(), &value, min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	void EditorLayer::Property(const std::string& name, bool& value)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		ImGui::Checkbox(id.c_str(), &value);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	void EditorLayer::OnImGuiRender()
	{
#define ENABLE_DOCKSPACE 1
#if ENABLE_DOCKSPACE
		static bool p_open = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &p_open, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		// Editor Panel ------------------------------------------------------------------------------


		ImGui::Begin("Model");
		if (ImGui::RadioButton("Spheres", (int*)&m_SceneType, (int)SceneType::Spheres))
			m_ActiveScene = m_SphereScene;
		ImGui::SameLine();
		if (ImGui::RadioButton("Model", (int*)&m_SceneType, (int)SceneType::Model))
			m_ActiveScene = m_Scene;

		ImGui::Begin("Environment");
		if (ImGui::Button("Load Environment Map"))
		{
			std::string filename = Application::Get().OpenFile("*.hdr");
			if (filename != "")
				m_ActiveScene->SetEnvironment(Environment::Load(filename));
		}

		ImGui::SliderFloat("Skybox LOD", &m_Scene->GetSkyboxLod(), 0.0f, 11.0f);
		ImGui::Columns(2);
		ImGui::AlignTextToFramePadding();

		Property("Light Direction", m_Light.Direction);
		Property("Light Radiance", m_Light.Radiance, PropertyFlag::ColorProperty);
		Property("Light Multiplier", m_LightMultiplier, 0.0f, 5.0f);
		Property("Exposure", m_ActiveScene->GetCamera().GetExposure(), 0.0f, 5.0f);

		Property("Radiance Prefiltering", m_RadiancePrefilter);
		Property("Env Map Rotation", m_EnvMapRotation, -360.0f, 360.0f);

		ImGui::Columns(1);

		ImGui::End();

		ImGui::Separator();
		{
			ImGui::Text("Mesh");
			auto mesh = m_MeshEntity->GetMesh();
			std::string fullpath = mesh ? mesh->GetFilePath() : "None";
			size_t found = fullpath.find_last_of("/\\");
			std::string path = found != std::string::npos ? fullpath.substr(found + 1) : fullpath;
			ImGui::Text(path.c_str()); ImGui::SameLine();
			if (ImGui::Button("...##Mesh"))
			{
				std::string filename = Prism::Application::Get().OpenFile("");
				if (filename != "")
				{
					auto newMesh = CreateRef<Mesh>(filename);
					// m_MeshMaterial.reset(new MaterialInstance(newMesh->GetMaterial()));
					// m_MeshEntity->SetMaterial(m_MeshMaterial);
					m_MeshEntity->SetMesh(newMesh);
				}
			}
		}
		ImGui::Separator();


		// Textures ------------------------------------------------------------------------------
		{
			// Albedo
			if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				ImGui::Image(m_AlbedoInput.TextureMap ? (void*)m_AlbedoInput.TextureMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered())
				{
					if (m_AlbedoInput.TextureMap)
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted(m_AlbedoInput.TextureMap->GetPath().c_str());
						ImGui::PopTextWrapPos();
						ImGui::Image((void*)m_AlbedoInput.TextureMap->GetRendererID(), ImVec2(384, 384));
						ImGui::EndTooltip();
					}
					if (ImGui::IsItemClicked())
					{
						std::string filename = Prism::Application::Get().OpenFile("");
						if (filename != "")
							m_AlbedoInput.TextureMap = Prism::Texture2D::Create(filename, m_AlbedoInput.SRGB);
					}
				}
				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::Checkbox("Use##AlbedoMap", &m_AlbedoInput.UseTexture);
				if (ImGui::Checkbox("sRGB##AlbedoMap", &m_AlbedoInput.SRGB))
				{
					if (m_AlbedoInput.TextureMap)
						m_AlbedoInput.TextureMap = Prism::Texture2D::Create(m_AlbedoInput.TextureMap->GetPath(), m_AlbedoInput.SRGB);
				}
				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(m_AlbedoInput.Color), ImGuiColorEditFlags_NoInputs);
			}
		}
		{
			// Normals
			if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				ImGui::Image(m_NormalInput.TextureMap ? (void*)m_NormalInput.TextureMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered())
				{
					if (m_NormalInput.TextureMap)
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted(m_NormalInput.TextureMap->GetPath().c_str());
						ImGui::PopTextWrapPos();
						ImGui::Image((void*)m_NormalInput.TextureMap->GetRendererID(), ImVec2(384, 384));
						ImGui::EndTooltip();
					}
					if (ImGui::IsItemClicked())
					{
						std::string filename = Prism::Application::Get().OpenFile("");
						if (filename != "")
							m_NormalInput.TextureMap = Prism::Texture2D::Create(filename);
					}
				}
				ImGui::SameLine();
				ImGui::Checkbox("Use##NormalMap", &m_NormalInput.UseTexture);
			}
		}
		{
			// Metalness
			if (ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				ImGui::Image(m_MetalnessInput.TextureMap ? (void*)m_MetalnessInput.TextureMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered())
				{
					if (m_MetalnessInput.TextureMap)
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted(m_MetalnessInput.TextureMap->GetPath().c_str());
						ImGui::PopTextWrapPos();
						ImGui::Image((void*)m_MetalnessInput.TextureMap->GetRendererID(), ImVec2(384, 384));
						ImGui::EndTooltip();
					}
					if (ImGui::IsItemClicked())
					{
						std::string filename = Prism::Application::Get().OpenFile("");
						if (filename != "")
							m_MetalnessInput.TextureMap = Prism::Texture2D::Create(filename);
					}
				}
				ImGui::SameLine();
				ImGui::Checkbox("Use##MetalnessMap", &m_MetalnessInput.UseTexture);
				ImGui::SameLine();
				ImGui::SliderFloat("Value##MetalnessInput", &m_MetalnessInput.Value, 0.0f, 1.0f);
			}
		}
		{
			// Roughness
			if (ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				ImGui::Image(m_RoughnessInput.TextureMap ? (void*)m_RoughnessInput.TextureMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered())
				{
					if (m_RoughnessInput.TextureMap)
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted(m_RoughnessInput.TextureMap->GetPath().c_str());
						ImGui::PopTextWrapPos();
						ImGui::Image((void*)m_RoughnessInput.TextureMap->GetRendererID(), ImVec2(384, 384));
						ImGui::EndTooltip();
					}
					if (ImGui::IsItemClicked())
					{
						std::string filename = Prism::Application::Get().OpenFile("");
						if (filename != "")
							m_RoughnessInput.TextureMap = Prism::Texture2D::Create(filename);
					}
				}
				ImGui::SameLine();
				ImGui::Checkbox("Use##RoughnessMap", &m_RoughnessInput.UseTexture);
				ImGui::SameLine();
				ImGui::SliderFloat("Value##RoughnessInput", &m_RoughnessInput.Value, 0.0f, 1.0f);
			}
		}

		ImGui::Separator();

		if (ImGui::TreeNode("Shaders"))
		{
			auto& shaders = Prism::PrismShader::s_AllShaders;
			for (auto& shader : shaders)
			{
				if (ImGui::TreeNode(shader->GetName().c_str()))
				{
					std::string buttonName = "Reload##" + shader->GetName();
					if (ImGui::Button(buttonName.c_str()))
						shader->Reload();
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		auto viewportSize = ImGui::GetContentRegionAvail();
		
		SceneRenderer::SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		m_ActiveScene->GetCamera().SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 10000.0f));
		m_ActiveScene->GetCamera().SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		ImGui::Image((void*)SceneRenderer::GetFinalColorBufferRendererID(), viewportSize, { 0, 1 }, { 1, 0 });
		//PR_TRACE("FrameBuffer {0}", m_FinalPresentBuffer->GetColorAttachmentRendererID());
		// Gizmos
		if (m_GizmoType != -1)
		{
			float rw = (float)ImGui::GetWindowWidth();
			float rh = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh);
			ImGuizmo::Manipulate(glm::value_ptr(m_ActiveScene->GetCamera().GetViewMatrix()), glm::value_ptr(m_ActiveScene->GetCamera().GetProjectionMatrix()), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(m_MeshEntity->Transform()));
		}
		ImGui::End();
		ImGui::PopStyleVar();
		

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
				ImGui::MenuItem("Padding", NULL, &opt_padding);
				ImGui::Separator();

				if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
				if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
				if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
				if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
				//if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
				ImGui::Separator();

				if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
					p_open = false;
				ImGui::EndMenu();
			}
			ImGuiShowHelpMarker(
				"When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
				"- Drag from window title bar or their tab to dock/undock." "\n"
				"- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
				"- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
				"- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)" "\n"
				"This demo app has nothing to do with enabling docking!" "\n\n"
				"This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window." "\n\n"
				"Read comments in ShowExampleAppDockSpace() for more details.");

			ImGui::EndMenuBar();
		}
		m_SceneHierarchyPanel->OnImGuiRender();
		ImGui::End();
#endif

		static bool show_demo_window = true;
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);
		//ImGui::ShowStyleEditor();
	}

	void EditorLayer::OnEvent(Prism::Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(PR_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
	}
	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
		case PR_KEY_Q:
			m_GizmoType = -1;
			break;
		case PR_KEY_W:
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case PR_KEY_E:
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case PR_KEY_R:
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}
		return false;
	}

}

