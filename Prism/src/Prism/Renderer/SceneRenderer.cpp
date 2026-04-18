#include "prpch.h"
#include "SceneRenderer.h"

#include "ComputeShader/ComputeShader.h"

#include "Shader/GlobalUniforms.h"
#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Prism
{
	static void UpdateGlobalsUBO();

	struct SceneRendererData
	{
		const Scene* ActiveScene = nullptr;
		struct SceneInfo
		{
			Camera SceneCamera;

			// Resources
			Ref<MaterialInstance> SkyboxMaterial;
			Environment SceneEnvironment;
			PrismGlobalsUBO SceneUniforms;

		} SceneData;

		Ref<Texture2D> BRDFLUT;
		Ref<PrismShader> CompositeShader;

		Ref<RenderPass> GeoPass;
		Ref<RenderPass> CompositePass;

		struct DrawCommand
		{
			Ref<Mesh> Mesh;
			Ref<MaterialInstance> Material;
			glm::mat4 Transform;
		};
		std::vector<DrawCommand> DrawList;

		// Grid
		Ref<MaterialInstance> GridMaterial;
	};

	static SceneRendererData s_Data;

	void SceneRenderer::Init()
	{
		FramebufferSpecification geoFramebufferSpec;
		geoFramebufferSpec.Width = 1280;
		geoFramebufferSpec.Height = 720;
		geoFramebufferSpec.Format = FramebufferFormat::RGBA16F;
		geoFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.TargetFramebuffer = Prism::Framebuffer::Create(geoFramebufferSpec);
		s_Data.GeoPass = RenderPass::Create(geoRenderPassSpec);

		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.Width = 1280;
		compFramebufferSpec.Height = 720;
		compFramebufferSpec.Format = FramebufferFormat::RGBA8;
		compFramebufferSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };

		RenderPassSpecification compRenderPassSpec;
		compRenderPassSpec.TargetFramebuffer = Prism::Framebuffer::Create(compFramebufferSpec);
		s_Data.CompositePass = RenderPass::Create(compRenderPassSpec);

		s_Data.CompositeShader = PrismShader::Create("Assets/Shaders/hdr.Shader");
		s_Data.BRDFLUT = Texture2D::Create("Assets/Textures/BRDF_LUT.tga");

		// Grid
		auto gridShader = PrismShader::Create("Assets/shaders/Grid.Shader");
		s_Data.GridMaterial = MaterialInstance::Create(Material::Create(gridShader));
		float gridScale = 16.025f, gridSize = 0.025f;
		s_Data.GridMaterial->Set("u_Scale", gridScale);
		s_Data.GridMaterial->Set("u_Res", gridSize);
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		s_Data.GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
		s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
	}

	void SceneRenderer::BeginScene(const Scene* scene)
	{
		PR_CORE_ASSERT(!s_Data.ActiveScene, "");

		s_Data.ActiveScene = scene;

		s_Data.SceneData.SceneCamera = scene->m_Camera;
		s_Data.SceneData.SkyboxMaterial = scene->m_SkyboxMaterial;
		s_Data.SceneData.SceneEnvironment = scene->m_Environment;

		UpdateGlobalsUBO();
	}

	void SceneRenderer::EndScene()
	{
		PR_CORE_ASSERT(s_Data.ActiveScene, "");

		s_Data.ActiveScene = nullptr;

		FlushDrawList();
	}

	void SceneRenderer::SubmitEntity(Entity* entity)
	{
		// TODO: Culling, sorting, etc.

		auto mesh = entity->GetMesh();
		if (!mesh)
			return;

		s_Data.DrawList.push_back({ mesh, entity->GetMaterial(), entity->GetTransform() });
	}

	static Ref<ComputeShader> environmentShader;


	std::pair<Ref<TextureCube>, Ref<TextureCube>> SceneRenderer::CreateEnvironmentMap(const std::string& filepath)
	{
		PR_PROFILE_FUNCTION();
		const uint32_t cubemapSize = 2048;
		const uint32_t irradianceMapSize = 32;

		Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
		if (!environmentShader)
			environmentShader = ComputeShader::Create("Assets/Shaders/Environment.compute");
		Ref<Texture2D> envEquirect = Texture2D::Create(filepath);
		PR_CORE_ASSERT(envEquirect->GetFormat() == TextureFormat::Float16, "Texture is not HDR!");

		envEquirect->Bind();
		int toCubeKernel = environmentShader->FindKernel("CSEquirectToCube");
		environmentShader->SetTexture2D(toCubeKernel, "u_EquirectangularTex", envEquirect);
		environmentShader->SetImageCube(toCubeKernel, "o_OutputCube", envUnfiltered);
		environmentShader->Dispatch(toCubeKernel, cubemapSize / 32, cubemapSize / 32, 6);
		//Renderer::MemoryBarriers(MBarrier::ImageAccess | MBarrier::TextureFetch);
		envUnfiltered->GenerateMipMap();

		Ref<TextureCube> envFiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
		envUnfiltered->CopyTo(envFiltered);
		//Renderer::MemoryBarriers(MBarrier::TextureUpdate | MBarrier::ImageAccess);

		int mipFilter = environmentShader->FindKernel("CSMipFilter");
		environmentShader->SetTextureCube(mipFilter, "u_InputCubeMap", envUnfiltered);
		const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
		for (int level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2)
		{
			const uint32_t numGroups = glm::max(1, size / 32);
			environmentShader->SetImageCube(mipFilter, "o_OutputCube", envFiltered, level, true);
			environmentShader->SetFloat(mipFilter,"u_Roughness", level * deltaRoughness);
			environmentShader->Dispatch(mipFilter, numGroups, numGroups, 6);
		}
		//Renderer::MemoryBarriers(MBarrier::ImageAccess | MBarrier::TextureFetch);

		Ref<TextureCube> irradianceMap = TextureCube::Create(TextureFormat::Float16, irradianceMapSize, irradianceMapSize);
		int irradiance = environmentShader->FindKernel("CSIrradiance");
		environmentShader->SetTextureCube(irradiance, "u_InputCubeMap", envFiltered);
		environmentShader->SetImageCube(irradiance, "o_OutputCube", irradianceMap);
		environmentShader->Dispatch(irradiance, irradianceMapSize / 32, irradianceMapSize / 32, 6);

		return { envFiltered, irradianceMap };
	}


	void SceneRenderer::GeometryPass()
	{
#if 1
		PR_PROFILE_FUNCTION();
		Renderer::BeginRenderPass(s_Data.GeoPass);

		auto viewProjection = s_Data.SceneData.SceneCamera.GetProjectionMatrix() * s_Data.SceneData.SceneCamera.GetViewMatrix();

		// Skybox
		auto skyboxShader = s_Data.SceneData.SkyboxMaterial->GetShader();
		s_Data.SceneData.SkyboxMaterial->Set("u_InverseVP", glm::inverse(viewProjection));
		// s_Data.SceneInfo.EnvironmentIrradianceMap->Bind(0);
		Renderer::SubmitFullscreenQuad(s_Data.SceneData.SkyboxMaterial);

		// Render entities
		for (auto& dc : s_Data.DrawList)
		{
			auto baseMaterial = dc.Mesh->GetMaterial();
			// Environment
			baseMaterial->Set("u_EnvRadianceTex", s_Data.SceneData.SceneEnvironment.RadianceMap);
			baseMaterial->Set("u_EnvIrradianceTex", s_Data.SceneData.SceneEnvironment.IrradianceMap);
			baseMaterial->Set("u_BRDFLUTTexture", s_Data.BRDFLUT);

			auto overrideMaterial = nullptr; // dc.Material;
			Renderer::SubmitMesh(dc.Mesh, dc.Transform, overrideMaterial);
		}
		Renderer::SubmitQuad(s_Data.GridMaterial, glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f)));

		Renderer::EndRenderPass();
#endif
	}

	void SceneRenderer::CompositePass()
	{
#if 1
		PR_PROFILE_FUNCTION();
		Renderer::BeginRenderPass(s_Data.CompositePass);
		s_Data.CompositeShader->Bind();
		s_Data.CompositeShader->SetFloat("u_Exposure", s_Data.SceneData.SceneCamera.GetExposure());
		s_Data.GeoPass->GetSpecification().TargetFramebuffer->BindTexture();
		Renderer::SubmitFullscreenQuad(nullptr);
		Renderer::EndRenderPass();
#endif
	}

	void SceneRenderer::FlushDrawList()
	{
		PR_CORE_ASSERT(!s_Data.ActiveScene, "");

		GeometryPass();
		CompositePass();

		s_Data.DrawList.clear();
		s_Data.SceneData = {};
	}

	Ref<Texture2D> SceneRenderer::GetFinalColorBuffer()
	{
		// return s_Data.CompositePass->GetSpecification().TargetFramebuffer;
		PR_CORE_ASSERT(false, "Not implemented");
		return nullptr;
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return s_Data.CompositePass->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID();
	}

	static void UpdateGlobalsUBO()
	{
		auto& camera = s_Data.SceneData.SceneCamera;
		auto& sceneUniforms = s_Data.SceneData.SceneUniforms;
		const auto& framebufferSpec = s_Data.GeoPass->GetSpecification().TargetFramebuffer->GetSpecification();
		sceneUniforms.AspectRatio = (float)framebufferSpec.Width / (float)framebufferSpec.Height;
		sceneUniforms.CameraPosition = camera.GetPosition();
		sceneUniforms.DeltaTime = Prism::Time::GetDeltaTime();
		sceneUniforms.Projection = camera.GetProjectionMatrix();
		sceneUniforms.View = camera.GetViewMatrix();
		sceneUniforms.ViewProjection = sceneUniforms.Projection * sceneUniforms.View;
		sceneUniforms.InverseViewProjection = glm::inverse(sceneUniforms.ViewProjection);
		float time = Prism::Time::GetTime();
		sceneUniforms.Time = glm::vec4(time * 0.2f, time, time * 2, time * 3);
		Renderer::Submit([=]() {
			Prism::GlobalUniforms::UpdateGlobalUniform(sceneUniforms);
			});
	}

}