#pragma once
#include <PrismShaderCore/CompilerCompute.h>
#include "Prism/Core/Ref.h"
#include "Prism/Asset/Asset.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace Prism
{
    class UniformBuffer;
    class ShaderStorageBuffer;
    class Image2D;
    class ImageCube;
    class Texture2D;
    class TextureCube;

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
        virtual void SetTexture2D(int32_t kernel, const std::string& name, Ref<Texture2D> texture) = 0;
        virtual void SetTextureCube(int32_t kernel, const std::string& name, Ref<TextureCube> texture) = 0;
        virtual void SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level = 0) = 0;
        virtual void SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level = 0) = 0;

        virtual void Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ, bool force = false) = 0;

        virtual void SetBool(int32_t kernel, const std::string& name, bool value) = 0;
        virtual void SetInt(int32_t kernel, const std::string& name, int32_t value) = 0;
        virtual void SetUInt(int32_t kernel, const std::string& name, uint32_t value) = 0;
        virtual void SetFloat(int32_t kernel, const std::string& name, float value) = 0;

        virtual void SetVector2(int32_t kernel, const std::string& name, const glm::vec2& value) = 0;
        virtual void SetVector3(int32_t kernel, const std::string& name, const glm::vec3& value) = 0;
        virtual void SetVector4(int32_t kernel, const std::string& name, const glm::vec4& value) = 0;

        virtual void SetIntVector2(int32_t kernel, const std::string& name, const glm::ivec2& value) = 0;
        virtual void SetIntVector3(int32_t kernel, const std::string& name, const glm::ivec3& value) = 0;
        virtual void SetIntVector4(int32_t kernel, const std::string& name, const glm::ivec4& value) = 0;

        virtual void SetUIntVector2(int32_t kernel, const std::string& name, const glm::uvec2& value) = 0;
        virtual void SetUIntVector3(int32_t kernel, const std::string& name, const glm::uvec3& value) = 0;
        virtual void SetUIntVector4(int32_t kernel, const std::string& name, const glm::uvec4& value) = 0;

        virtual void SetBoolVector2(int32_t kernel, const std::string& name, const glm::bvec2& value) = 0;
        virtual void SetBoolVector3(int32_t kernel, const std::string& name, const glm::bvec3& value) = 0;
        virtual void SetBoolVector4(int32_t kernel, const std::string& name, const glm::bvec4& value) = 0;

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
        int32_t FindUniform(const std::string& name) const;
        virtual bool IsLegalKernel(int32_t kernel) const = 0;

        std::vector<Slot> m_Slots;

        PrismShaderCompiler::CompiledComputeShader m_Compiled;

        std::string m_Name;
        std::string m_FilePath;
    };
}
