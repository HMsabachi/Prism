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
        virtual void Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ, bool force = false) override;

        virtual void SetBool(int32_t kernel, const std::string& name, bool value) override;
        virtual void SetInt(int32_t kernel, const std::string& name, int32_t value) override;
        virtual void SetUInt(int32_t kernel, const std::string& name, uint32_t value) override;
        virtual void SetFloat(int32_t kernel, const std::string& name, float value) override;

        virtual void SetVector2(int32_t kernel, const std::string& name, const glm::vec2& value) override;
        virtual void SetVector3(int32_t kernel, const std::string& name, const glm::vec3& value) override;
        virtual void SetVector4(int32_t kernel, const std::string& name, const glm::vec4& value) override;

        virtual void SetIntVector2(int32_t kernel, const std::string& name, const glm::ivec2& value) override;
        virtual void SetIntVector3(int32_t kernel, const std::string& name, const glm::ivec3& value) override;
        virtual void SetIntVector4(int32_t kernel, const std::string& name, const glm::ivec4& value) override;

        virtual void SetUIntVector2(int32_t kernel, const std::string& name, const glm::uvec2& value) override;
        virtual void SetUIntVector3(int32_t kernel, const std::string& name, const glm::uvec3& value) override;
        virtual void SetUIntVector4(int32_t kernel, const std::string& name, const glm::uvec4& value) override;

        virtual void SetBoolVector2(int32_t kernel, const std::string& name, const glm::bvec2& value) override;
        virtual void SetBoolVector3(int32_t kernel, const std::string& name, const glm::bvec3& value) override;
        virtual void SetBoolVector4(int32_t kernel, const std::string& name, const glm::bvec4& value) override;

    protected:
        virtual bool IsLegalKernel(int32_t kernel) const override;

    private:
        void WriteUniform(int32_t kernel, const std::string& name, PrismShaderCompiler::GLSLType type,
                          const void* data, uint32_t size);

        class Kernel
        {
        public:
            std::string Name;
            uint32_t GroupSizeX = 1;
            uint32_t GroupSizeY = 1;
            uint32_t GroupSizeZ = 1;
            Ref<VulkanShader> Shader;
            VulkanDescriptorSet Set;

            // 本 kernel 的 push constant 值（按 m_Compiled.Uniforms 的 Offset 排布）
            std::vector<uint8_t> UniformData;
        };

        std::vector<Kernel> m_Kernels;
    };
}
