#pragma once

#include "Prism/Renderer/RendererAPI.h"

namespace Prism
{

    class OpenGLRenderer : public RendererAPI
    {
    public:
        virtual void Init() override;
        virtual void Shutdown() override;

        virtual void BeginFrame() override;
        virtual void EndFrame() override;

        virtual void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true) override;
        virtual void EndRenderPass() override;

        virtual void SubmitFullscreenQuad(Ref<VertexInput> vertexInput, Ref<Material> material,
            const PrismShaderCompiler::PipelineState* stateOverride = nullptr) override;

        virtual void SetSceneEnvironment(const Ref<SceneEnvironment>& environment) override;
        virtual std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath) override;

        virtual void SetUniformBuffer(uint32_t set, uint32_t binding, Ref<UniformBuffer> ubo) override;
        virtual void SetShaderStorageBuffer(uint32_t set, uint32_t binding, Ref<ShaderStorageBuffer> ssbo) override;
        virtual void SetTexture(uint32_t set, uint32_t binding, Ref<Image> image) override;

        virtual void RenderMesh(Ref<Mesh> mesh, Ref<Material> material,
            uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass,
            const PrismShaderCompiler::PipelineState* stateOverride = nullptr) override;
        virtual void RenderQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const glm::mat4& transform,
            const PrismShaderCompiler::PipelineState* stateOverride = nullptr) override;

        virtual void DispatchCompute(Ref<Shader> kernelShader,
            const std::vector<ComputeResourceBinding>& bindings,
            uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) override;

        virtual RenderAPICapabilities& GetCapabilities() override;

    private:
        void RT_BindMaterial(Ref<Material> material, uint32_t pass, const PrismShaderCompiler::PipelineState* stateOverride = nullptr);

        static uint32_t FlatUBO(uint32_t set, uint32_t binding);
        static uint32_t FlatSSBO(uint32_t set, uint32_t binding);
        static uint32_t FlatTexture(uint32_t set, uint32_t binding);
    };
}
