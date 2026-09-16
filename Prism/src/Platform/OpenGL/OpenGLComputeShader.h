#pragma once

#include "Prism/Renderer/ComputeShader/ComputeShader.h"

namespace Prism
{
    class OpenGLComputeShader : public ComputeShader
    {
    public:
        OpenGLComputeShader(const std::string& filePath);

        virtual void SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo) override;
        virtual void SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo) override;
        virtual void SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image) override;
        virtual void SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image) override;
        virtual void SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level = 0) override;
        virtual void SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level = 0) override;
        virtual void Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;

    private:
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

        std::vector<KernelValues> m_KernelValues;
    };
}
