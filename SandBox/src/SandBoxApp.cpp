#include <Prism.h>

#include "imgui/imgui.h"
#include "Prism/Core/EntryPoint.h"
#include "glm/gtc/type_ptr.inl"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>



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

class EditorLayer : public Prism::Layer
{
public:
	EditorLayer()
		: m_Scene(Scene::Spheres),
		m_Camera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f))
	{
	}


	virtual ~EditorLayer()
	{
	}
	void LoadTexture(Prism::Ref<Prism::PrismShader> shader)
	{
		using tex = Prism::Texture2DType;
		using texc = Prism::TextureCubeType;
		Prism::UniformBufferDeclaration<sizeof(int) * 8, 8> decl;
		// Bind default texture unit
		auto& pro = shader->GetProperties();
		decl.Push("u_Texture", pro["u_Texture"].GetDefaultValue<tex>().id);

		// PBR shader Textures
		decl.Push("u_AlbedoTexture", pro["u_AlbedoTexture"].GetDefaultValue<tex>().id);
		decl.Push("u_NormalTexture", pro["u_NormalTexture"].GetDefaultValue<tex>().id);
		decl.Push("u_MetalnessTexture", pro["u_MetalnessTexture"].GetDefaultValue<tex>().id);
		decl.Push("u_RoughnessTexture", pro["u_RoughnessTexture"].GetDefaultValue<tex>().id);

		decl.Push("u_EnvRadianceTex", pro["u_EnvRadianceTex"].GetDefaultValue<texc>().id);
		decl.Push("u_EnvIrradianceTex", pro["u_EnvIrradianceTex"].GetDefaultValue<texc>().id);

		decl.Push("u_BRDFLUTTexture", pro["u_BRDFLUTTexture"].GetDefaultValue<tex>().id);

		shader->GetOriginalShader()->UploadUniformBuffer(decl);
	}
	virtual void OnAttach() override
	{
		Prism::GlobalUniforms::Init();

		m_SimplePBRPrismShader = Prism::PrismShader::Create("Assets/Shaders/simplepbrPrism.glsl");
		PR_TRACE(*m_SimplePBRPrismShader);
		//LoadTexture(m_SimplePBRPrismShader);
		m_SimplePBRShader = m_SimplePBRPrismShader->GetOriginalShader();
		//m_SimplePBRShader.reset(Prism::Shader::Create("Assets/Shaders/simplepbr.glsl"));
		m_QuadShader.reset(Prism::Shader::Create("Assets/Shaders/quad.glsl"));
		m_HDRShader.reset(Prism::Shader::Create("Assets/Shaders/hdr.glsl"));
		m_Mesh.reset(new Prism::Mesh("Assets/meshes/cerberus.fbx"));
		m_SphereMesh.reset(new Prism::Mesh("Assets/models/Sphere.fbx"));

		// Editor
		m_CheckerboardTex.reset(Prism::Texture2D::Create("Assets/editor/Checkerboard.tga"));

		// Environment
		m_EnvironmentCubeMap.reset(Prism::TextureCube::Create("Assets/Textures/environments/Arches_E_PineTree_Radiance.tga"));
		//m_EnvironmentCubeMap.reset(Prism::TextureCube::Create("Assets/Textures/environments/DebugCubeMap.tga"));
		m_EnvironmentIrradiance.reset(Prism::TextureCube::Create("Assets/Textures/environments/Arches_E_PineTree_Irradiance.tga"));
		m_BRDFLUT.reset(Prism::Texture2D::Create("Assets/Textures/BRDF_LUT.tga"));

		m_Framebuffer.reset(Prism::Framebuffer::Create(1280, 720, Prism::FramebufferFormat::RGBA16F));
		m_FinalPresentBuffer.reset(Prism::Framebuffer::Create(1280, 720, Prism::FramebufferFormat::RGBA8));

		// Create Quad
		float x = -1, y = -1;
		float width = 2, height = 2;
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];

		data[0].Position = glm::vec3(x, y, 0);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0);
		data[3].TexCoord = glm::vec2(0, 1);

		m_VertexBuffer.reset(Prism::VertexBuffer::Create());
		m_VertexBuffer->SetData(data, 4 * sizeof(QuadVertex));

		uint32_t* indices = new uint32_t[6]{ 0, 1, 2, 2, 3, 0, };
		m_IndexBuffer.reset(Prism::IndexBuffer::Create());
		m_IndexBuffer->SetData(indices, 6 * sizeof(unsigned int));

		m_Light.Direction = { -0.5f, -0.5f, 1.0f };
		m_Light.Radiance = { 1.0f, 1.0f, 1.0f };
	}

	virtual void OnDetach() override
	{
	}

	virtual void OnUpdate() override
	{
		UpdateGlobalsUBO();
		// THINGS TO LOOK AT:
		// - BRDF LUT
		// - Cubemap mips and filtering
		// - Tonemapping and proper HDR pipeline
		using namespace Prism;
		using namespace glm;

		m_Camera.Update();
		auto viewProjection = m_Camera.GetProjectionMatrix() * m_Camera.GetViewMatrix();

		m_Framebuffer->Bind();
		Renderer::Clear();

		
		Prism::UniformBufferDeclaration<sizeof(mat4), 1> quadShaderUB;
		quadShaderUB.Push("u_InverseVP", inverse(viewProjection));
		m_QuadShader->UploadUniformBuffer(quadShaderUB);

		m_QuadShader->Bind();
		m_EnvironmentCubeMap->Bind(0);
		m_VertexBuffer->Bind();
		m_IndexBuffer->Bind();
		Renderer::DrawIndexed(m_IndexBuffer->GetCount(), false);

		Prism::UniformBufferDeclaration<sizeof(mat4) * 3 + sizeof(vec3) * 4 + sizeof(float) * 8, 15> simplePbrShaderUB;
		simplePbrShaderUB.Push("u_ViewProjectionMatrix", viewProjection);
		simplePbrShaderUB.Push("u_ModelMatrix", mat4(1.0f));
		simplePbrShaderUB.Push("Prism_Model", mat4(1.0f));
		simplePbrShaderUB.Push("u_AlbedoColor", m_AlbedoInput.Color);
		simplePbrShaderUB.Push("u_Metalness", m_MetalnessInput.Value);
		simplePbrShaderUB.Push("u_Roughness", m_RoughnessInput.Value);
		simplePbrShaderUB.Push("lights.Direction", m_Light.Direction);
		simplePbrShaderUB.Push("lights.Radiance", m_Light.Radiance * m_LightMultiplier);
		simplePbrShaderUB.Push("u_CameraPosition", m_Camera.GetPosition());
		simplePbrShaderUB.Push("u_RadiancePrefilter", m_RadiancePrefilter ? 1.0f : 0.0f);
		simplePbrShaderUB.Push("u_AlbedoTexToggle", m_AlbedoInput.UseTexture ? 1.0f : 0.0f);
		simplePbrShaderUB.Push("u_NormalTexToggle", m_NormalInput.UseTexture ? 1.0f : 0.0f);
		simplePbrShaderUB.Push("u_MetalnessTexToggle", m_MetalnessInput.UseTexture ? 1.0f : 0.0f);
		simplePbrShaderUB.Push("u_RoughnessTexToggle", m_RoughnessInput.UseTexture ? 1.0f : 0.0f);
		simplePbrShaderUB.Push("u_EnvMapRotation", m_EnvMapRotation);
		m_SimplePBRShader->UploadUniformBuffer(simplePbrShaderUB);

		m_EnvironmentCubeMap->Bind(10);
		m_EnvironmentIrradiance->Bind(11);
		m_BRDFLUT->Bind(15);

		m_SimplePBRShader->Bind();
		if (m_AlbedoInput.TextureMap)
			m_AlbedoInput.TextureMap->Bind(1);
		if (m_NormalInput.TextureMap)
			m_NormalInput.TextureMap->Bind(2);
		if (m_MetalnessInput.TextureMap)
			m_MetalnessInput.TextureMap->Bind(3);
		if (m_RoughnessInput.TextureMap)
			m_RoughnessInput.TextureMap->Bind(4);

		if (m_Scene == Scene::Spheres)
		{
			// Metals
			float roughness = 0.0f;
			float x = -88.0f;
			for (int i = 0; i < 8; i++)
			{
				m_SimplePBRShader->SetMat4("u_ModelMatrix", translate(mat4(1.0f), vec3(x, 0.0f, 0.0f)));
				m_SimplePBRShader->SetMat4("Prism_Model", translate(mat4(1.0f), vec3(x, 0.0f, 0.0f)));
				m_SimplePBRShader->SetFloat("u_Roughness", roughness);
				m_SimplePBRShader->SetFloat("u_Metalness", 1.0f);
				m_SphereMesh->Render();

				roughness += 0.15f;
				x += 22.0f;
			}

			// Dielectrics
			roughness = 0.0f;
			x = -88.0f;
			for (int i = 0; i < 8; i++)
			{
				m_SimplePBRShader->SetMat4("u_ModelMatrix", translate(mat4(1.0f), vec3(x, 22.0f, 0.0f)));
				m_SimplePBRShader->SetMat4("Prism_Model", translate(mat4(1.0f), vec3(x, 22.0f, 0.0f)));
				m_SimplePBRShader->SetFloat("u_Roughness", roughness);
				m_SimplePBRShader->SetFloat("u_Metalness", 0.0f);
				m_SphereMesh->Render();

				roughness += 0.15f;
				x += 22.0f;
			}

		}
		else if (m_Scene == Scene::Model)
		{
			m_Mesh->Render();
		}

		m_Framebuffer->Unbind();

		/*m_FinalPresentBuffer->Bind();
		m_HDRShader->Bind();
		m_HDRShader->SetFloat("u_Exposure", m_Exposure);
		m_Framebuffer->BindTexture();
		m_VertexBuffer->Bind();
		m_IndexBuffer->Bind();
		Renderer::DrawIndexed(m_IndexBuffer->GetCount(), false);
		m_FinalPresentBuffer->Unbind();*/
	}

	enum class PropertyFlag
	{
		None = 0, ColorProperty = 1
	};

	void Property(const std::string& name, bool& value)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		ImGui::Checkbox(id.c_str(), &value);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	void Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		ImGui::SliderFloat(id.c_str(), &value, min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	void Property(const std::string& name, glm::vec3& value, PropertyFlag flags)
	{
		Property(name, value, -1.0f, 1.0f, flags);
	}

	void Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None)
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

	void Property(const std::string& name, glm::vec4& value, PropertyFlag flags)
	{
		Property(name, value, -1.0f, 1.0f, flags);
	}

	void Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None)
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

	virtual void OnImGuiRender() override
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

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
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
		ImGui::RadioButton("Spheres", (int*)&m_Scene, (int)Scene::Spheres);
		ImGui::SameLine();
		ImGui::RadioButton("Model", (int*)&m_Scene, (int)Scene::Model);

		ImGui::Begin("Environment");
		ImGui::Columns(2);
		ImGui::AlignTextToFramePadding();

		Property("Light Direction", m_Light.Direction);
		Property("Light Radiance", m_Light.Radiance, PropertyFlag::ColorProperty);
		Property("Light Multiplier", m_LightMultiplier, 0.0f, 5.0f);
		Property("Exposure", m_Exposure, 0.0f, 5.0f);

		Property("Radiance Prefiltering", m_RadiancePrefilter);
		Property("Env Map Rotation", m_EnvMapRotation, -360.0f, 360.0f);

		ImGui::Columns(1);

		ImGui::End();

		ImGui::Separator();
		{
			ImGui::Text("Mesh");
			std::string fullpath = m_Mesh ? m_Mesh->GetFilePath() : "None";
			size_t found = fullpath.find_last_of("/\\");
			std::string path = found != std::string::npos ? fullpath.substr(found + 1) : fullpath;
			ImGui::Text(path.c_str()); ImGui::SameLine();
			if (ImGui::Button("...##Mesh"))
			{
				std::string filename = Prism::Application::Get().OpenFile("");
				if (filename != "")
					m_Mesh.reset(new Prism::Mesh(filename));
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
							m_AlbedoInput.TextureMap.reset(Prism::Texture2D::Create(filename, m_AlbedoInput.SRGB));
					}
				}
				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::Checkbox("Use##AlbedoMap", &m_AlbedoInput.UseTexture);
				if (ImGui::Checkbox("sRGB##AlbedoMap", &m_AlbedoInput.SRGB))
				{
					if (m_AlbedoInput.TextureMap)
						m_AlbedoInput.TextureMap.reset(Prism::Texture2D::Create(m_AlbedoInput.TextureMap->GetPath(), m_AlbedoInput.SRGB));
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
							m_NormalInput.TextureMap.reset(Prism::Texture2D::Create(filename));
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
							m_MetalnessInput.TextureMap.reset(Prism::Texture2D::Create(filename));
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
							m_RoughnessInput.TextureMap.reset(Prism::Texture2D::Create(filename));
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
			auto& shaders = Prism::Shader::s_AllShaders;
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
		m_Framebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		m_FinalPresentBuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		m_Camera.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 10000.0f));
		ImGui::Image((void*)m_Framebuffer->GetColorAttachmentRendererID(), viewportSize, { 0, 1 }, { 1, 0 });
		//PR_TRACE("FrameBuffer {0}", m_FinalPresentBuffer->GetColorAttachmentRendererID());
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
				if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
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

		ImGui::End();
#endif

		static bool show_demo_window = true;
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);
	}

	virtual void OnEvent(Prism::Event& event) override
	{
	}

private:
	void UpdateGlobalsUBO()
	{
		m_GlobalsUBO.AspectRatio = (float)m_Framebuffer->GetWidth() / (float)m_Framebuffer->GetHeight();
		m_GlobalsUBO.CameraPosition = m_Camera.GetPosition();
		m_GlobalsUBO.DeltaTime = Prism::Time::GetDeltaTime();
		m_GlobalsUBO.Projection = m_Camera.GetProjectionMatrix();
		m_GlobalsUBO.View = m_Camera.GetViewMatrix();
		m_GlobalsUBO.ViewProjection = m_GlobalsUBO.Projection * m_GlobalsUBO.View;
		float time = Prism::Time::GetTime();
		m_GlobalsUBO.Time = glm::vec4(time * 0.2f, time, time * 2, time * 3);
		PR_RENDER_S({
			Prism::GlobalUniforms::UpdateGlobalUniform(self->m_GlobalsUBO);
			});
		

	}
private:
	Prism::PrismGlobalsUBO m_GlobalsUBO;
	Prism::Ref<Prism::PrismShader> m_SimplePBRPrismShader;
	std::unique_ptr<Prism::Shader> m_Shader;
	std::unique_ptr<Prism::Shader> m_PBRShader;
	Prism::Ref<Prism::Shader> m_SimplePBRShader;
	std::unique_ptr<Prism::Shader> m_QuadShader;
	std::unique_ptr<Prism::Shader> m_HDRShader;
	std::unique_ptr<Prism::Mesh> m_Mesh;
	std::unique_ptr<Prism::Mesh> m_SphereMesh;
	std::unique_ptr<Prism::Texture2D> m_BRDFLUT;

	struct AlbedoInput
	{
		glm::vec3 Color = { 0.972f, 0.96f, 0.915f }; // Silver, from https://docs.unrealengine.com/en-us/Engine/Rendering/Materials/PhysicallyBased
		std::unique_ptr<Prism::Texture2D> TextureMap;
		bool SRGB = true;
		bool UseTexture = false;
	};
	AlbedoInput m_AlbedoInput;

	struct NormalInput
	{
		std::unique_ptr<Prism::Texture2D> TextureMap;
		bool UseTexture = false;
	};
	NormalInput m_NormalInput;

	struct MetalnessInput
	{
		float Value = 1.0f;
		std::unique_ptr<Prism::Texture2D> TextureMap;
		bool UseTexture = false;
	};
	MetalnessInput m_MetalnessInput;

	struct RoughnessInput
	{
		float Value = 0.5f;
		std::unique_ptr<Prism::Texture2D> TextureMap;
		bool UseTexture = false;
	};
	RoughnessInput m_RoughnessInput;

	std::unique_ptr<Prism::Framebuffer> m_Framebuffer, m_FinalPresentBuffer;

	std::unique_ptr<Prism::VertexBuffer> m_VertexBuffer;
	std::unique_ptr<Prism::IndexBuffer> m_IndexBuffer;
	std::unique_ptr<Prism::TextureCube> m_EnvironmentCubeMap, m_EnvironmentIrradiance;

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

class Sandbox : public Prism::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		//PushLayer(new Sandbox2D());
		PushLayer(new EditorLayer());
		
	}
	~Sandbox()
	{

	}
};

Prism::Application* Prism::CreateApplication()
{
	return new Sandbox();
}