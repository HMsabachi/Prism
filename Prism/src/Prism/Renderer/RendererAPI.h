#pragma once
#include "RendererTypes.h"
#include "Prism/Utilities/BitFlags.h"
#include "Prism/Core/Ref.h"
#include "Prism/ShaderCompiler/PrismBindings.h"

#include <string>
#include <utility>
#include <vector>
#include <glm/glm.hpp>

namespace PrismShaderCompiler { struct PipelineState; }

namespace Prism
{
    class RenderPass;
    class Mesh;
    class Material;
    class TextureCube;
    class Image;
    class Image2D;
    class SceneEnvironment;
    class Shader;
    class UniformBuffer;
    class ShaderStorageBuffer;
    class Texture;

    using RendererID = uint32_t;

    enum class RendererAPIType
    {
        None = 0,
        OpenGL = 1
    };

    enum class ComputeBindingKind
    {
        None = 0,
        UniformBuffer,
        StorageBuffer,
        Sampler,
        Image
    };

    struct ComputeResourceBinding
    {
        ComputeBindingKind Kind = ComputeBindingKind::None;
        uint32_t Binding = 0;
        bool ReadOnly = false;
        bool WriteOnly = false;
        bool Layered = true;
        uint32_t Level = 0;
        Ref<UniformBuffer> UBO;
        Ref<ShaderStorageBuffer> SSBO;
        Ref<Texture> Texture;
    };

    struct RenderAPICapabilities
    {
        std::string Vendor;
        std::string Renderer;
        std::string Version;

        int MaxSamples = 0;
        float MaxAnisotropy = 0.0f;
        int MaxTextureUnits = 0;

        int MaxGroupCount[3]{}, MaxGroupSize[3]{}, MaxInvocations{};
    };

    class PRISM_API RendererAPI
    {
    public:
#pragma region 数据结构
        enum class PRISM_API Barrier
        {
            None = 0,
            ShaderStorage = BIT(0),
            VertexAttribArray = BIT(1),
            ElementArray = BIT(2),
            ImageAccess = BIT(3),
            TextureFetch = BIT(4),
            TextureUpdate = BIT(5),
            Framebuffer = BIT(6),
            Command = BIT(7),
            All = BIT(8)
        };
        typedef BitFlags<Barrier> BarrierFlags;
#pragma endregion

    public:
        virtual ~RendererAPI() = default;

        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true) = 0;
        virtual void EndRenderPass() = 0;
        virtual void SubmitFullscreenQuad(Ref<Material> material, const PrismShaderCompiler::PipelineState* stateOverride = nullptr) = 0;

        virtual void SetSceneEnvironment(const Ref<SceneEnvironment>& environment) = 0;
        virtual std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath) = 0;

        virtual void SetUniformBuffer(uint32_t set, uint32_t binding, Ref<UniformBuffer> ubo) = 0;
        virtual void SetShaderStorageBuffer(uint32_t set, uint32_t binding, Ref<ShaderStorageBuffer> ssbo) = 0;
        virtual void SetTexture(uint32_t set, uint32_t binding, Ref<Image> image) = 0;

        virtual void RenderMesh(Ref<Mesh> mesh, Ref<Material> material,
            uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass,
            const PrismShaderCompiler::PipelineState* stateOverride = nullptr) = 0;
        virtual void RenderQuad(Ref<Material> material, const glm::mat4& transform,
            const PrismShaderCompiler::PipelineState* stateOverride = nullptr) = 0;

        virtual void DispatchCompute(Ref<Shader> kernelShader,
            const std::vector<ComputeResourceBinding>& bindings,
            uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) = 0;

        virtual RenderAPICapabilities& GetCapabilities() = 0;

        static RendererAPIType Current() { return s_CurrentRendererAPI; }
    private:
        static RendererAPIType s_CurrentRendererAPI;
    };
    using MBarrier = RendererAPI::Barrier;
}
