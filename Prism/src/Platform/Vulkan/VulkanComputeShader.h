#pragma once

#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "VulkanDescriptorSet.h"

namespace Prism
{
    class VulkanShader;

    class VulkanComputeShader : public ComputeShader
    {
    public:
        VulkanComputeShader(const std::string& filePath);

        virtual void SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo) override;
        virtual void SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo) override;
        virtual void SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image) override;
        virtual void SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image) override;
        virtual void SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level = 0) override;
        virtual void SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level = 0) override;
        virtual void Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;

    private:
        class KernelResources : public RefCounted
        {
        public:
            VulkanDescriptorSet Set;
            Ref<VulkanShader> Shader;
        };

        struct SlotValue
        {
            Ref<RefCounted> Resource;
            uint32_t Level = 0;
        };

        struct KernelValues
        {
            std::vector<SlotValue> Slots;
        };

        void SetSlot(int32_t kernel, const std::string& name,
            PrismShaderCompiler::CSL::ResourceKind kind, Ref<RefCounted> resource, uint32_t level);

        std::vector<Ref<KernelResources>> m_KernelResources;
        std::vector<KernelValues> m_KernelValues;
    };
}
