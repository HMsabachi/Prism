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

        virtual int32_t FindKernel(const std::string& name) const override;
        virtual bool HasKernel(const std::string& name) const override;
        virtual void GetKernelThreadGroupSizes(int32_t kernel, uint32_t& x, uint32_t& y, uint32_t& z) const override;
        virtual size_t GetKernelCount() const override { return m_Kernels.size(); }

        virtual void SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo) override;
        virtual void SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo) override;
        virtual void SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image) override;
        virtual void SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image) override;
        virtual void SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level = 0) override;
        virtual void SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level = 0) override;
        virtual void Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;

    protected:
        virtual bool IsLegalKernel(int32_t kernel) const override;

    private:
        class Kernel : public RefCounted
        {
        public:
            std::string Name;
            uint32_t GroupSizeX = 1;
            uint32_t GroupSizeY = 1;
            uint32_t GroupSizeZ = 1;
            Ref<VulkanShader> Shader;
            VulkanDescriptorSet Set;
        };

        std::vector<Ref<Kernel>> m_Kernels;
    };
}
