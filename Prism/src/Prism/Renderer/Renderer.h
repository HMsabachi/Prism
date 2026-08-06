#pragma once

#include "RendererTypes.h"
#include "RendererAPI.h"
#include "RenderCommandQueue.h"

#include <glm/glm.hpp>
#include "Prism/Core/Math/AABB.h"

namespace Prism
{
    class Camera;
    class Mesh;
    class Material;
    class RenderPass;
    class VertexInput;
    class Image;
}

namespace Prism
{
    class PRISM_API Renderer
    {
    private:

    public:
        typedef void(*RenderCommandFn)(void*);
        Renderer();
        ~Renderer();
        static void* DataAllocate(const void* data, size_t size);
        template<typename FuncT>
        static void Submit(FuncT&& func)
        {
            auto renderCmd = [](void* ptr) {
                auto pFunc = (FuncT*)ptr;
                (*pFunc)();
                pFunc->~FuncT();
                };
            auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
            new (storageBuffer) FuncT(std::forward<FuncT>(func));
            //renderCmd(&func);
        }
        static void Init();
        static void Shutdown();

        // 高层转发（多态分发到 s_RendererAPI）
        static void BeginFrame();
        static void EndFrame();

        static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true);
        static void EndRenderPass();
        static void SubmitFullscreenQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const PrismShaderCompiler::PipelineState* stateOverride = nullptr);
        static void SetSceneEnvironment(const Ref<SceneEnvironment>& environment);
        static void SetGlobalTexture(uint32_t slot, Ref<Image> image);
        static std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath);
        static void RenderMesh(Ref<VertexInput> vertexInput, Ref<Mesh> mesh, Ref<Material> material, uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass, const PrismShaderCompiler::PipelineState* stateOverride = nullptr);
        static void RenderQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const glm::mat4& transform, const PrismShaderCompiler::PipelineState* stateOverride = nullptr);

        static void DispatchCompute(Ref<Shader> kernelShader, const std::vector<ComputeResourceBinding>& bindings, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);

        static RenderAPICapabilities& GetCapabilities();

        static void WaitAndRender();

#if 0
        // 依赖 Renderer2D::DrawLine，待跟进 Hazel SceneRenderer 线框方案后恢复
        static void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f,0.0f,0.0f,1.0f));
        static void DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
#endif
    public:
        static RenderCommandQueue& GetRenderCommandQueue();
    };
}
