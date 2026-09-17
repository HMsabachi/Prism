#pragma once
#include <PrismShaderCore/CompilerCompute.h>
#include "Prism/Core/Ref.h"
#include "Prism/Asset/Asset.h"

#include <string>
#include <vector>

namespace Prism
{
    class UniformBuffer;
    class ShaderStorageBuffer;
    class Image2D;
    class ImageCube;

    class ComputeShader : public Asset
    {
    public:
        static Ref<ComputeShader> Create(const std::string& filePath);
        static Ref<ComputeShader> Create(AssetHandle handle);

        virtual ~ComputeShader() = default;

        const std::string& GetName() const { return m_Name; }
        const std::string& GetFilePath() const { return m_FilePath; }

        virtual int32_t FindKernel(const std::string& name) const = 0;
        virtual bool HasKernel(const std::string& name) const = 0;
        virtual void GetKernelThreadGroupSizes(int32_t kernel, uint32_t& x, uint32_t& y, uint32_t& z) const = 0;
        virtual size_t GetKernelCount() const = 0;

        virtual void SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo) = 0;
        virtual void SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo) = 0;
        virtual void SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image) = 0;
        virtual void SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image) = 0;
        virtual void SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level = 0) = 0;
        virtual void SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level = 0) = 0;

        virtual void Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) = 0;

    protected:
        ComputeShader(const std::string& filePath);
        void Load();

        struct Slot
        {
            uint32_t Set = 0;
            uint32_t Binding = 0;
            PrismShaderCompiler::CSL::ResourceKind Kind = PrismShaderCompiler::CSL::ResourceKind::StorageBuffer;
            bool ReadOnly = false;
            bool WriteOnly = false;
            std::string Name;
        };

        int32_t FindSlot(const std::string& name, PrismShaderCompiler::CSL::ResourceKind expected) const;
        virtual bool IsLegalKernel(int32_t kernel) const = 0;

        std::vector<Slot> m_Slots;

        PrismShaderCompiler::CompiledComputeShader m_Compiled;

        std::string m_Name;
        std::string m_FilePath;
    };
}
