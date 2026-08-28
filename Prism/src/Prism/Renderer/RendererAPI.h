#pragma once
#include "RendererTypes.h"
#include "Prism/Core/Ref.h"
#include "Prism/ShaderCompiler/PrismBindings.h"

#include <string>
#include <utility>
#include <vector>
#include <glm/glm.hpp>

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
    class ComputeShader;
    class UniformBuffer;
    class ShaderStorageBuffer;
    class Texture;

    using RendererID = uint32_t;

    enum class RendererAPIType : uint8_t
    {
        None = 0,
        OpenGL = 1,
        Vulkan = 2
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
        virtual ~RendererAPI() = default;

        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true) = 0;
        virtual void EndRenderPass() = 0;
        virtual void SubmitFullscreenQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex = 0) = 0;

        virtual void SetGlobalUniformBuffer(uint32_t binding, Ref<UniformBuffer> ubo) = 0;
        virtual void SetGlobalShaderStorageBuffer(uint32_t binding, Ref<ShaderStorageBuffer> ssbo) = 0;
        virtual void SetGlobalTexture(uint32_t binding, Ref<Image> image) = 0;
        virtual void BakeGlobalInputs() = 0;

        virtual void RenderMesh(Ref<Mesh> mesh, uint32_t submeshIndex, Ref<Material> material,
            uint32_t passIndex, uint32_t drawIndex = 0) = 0;
        virtual void RenderQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex = 0) = 0;

        virtual void DispatchCompute(Ref<ComputeShader> computeShader, int32_t kernel,
            uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) = 0;

        virtual RenderAPICapabilities& GetCapabilities() = 0;

        static RendererAPIType Current() { return s_CurrentRendererAPI; }
        static void SetCurrent(RendererAPIType api) { PR_CORE_ASSERT(s_CurrentRendererAPI == RendererAPIType::None); s_CurrentRendererAPI = api; }
    private:
        static RendererAPIType s_CurrentRendererAPI;
    };
}
