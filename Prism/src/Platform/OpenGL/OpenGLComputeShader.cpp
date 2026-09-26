#include "prpch.h"
#include "OpenGLComputeShader.h"

#include "OpenGLShader.h"
#include "OpenGLImage.h"
#include "OpenGLUniformBuffer.h"
#include "OpenGLShaderStorageBuffer.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Shader.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"

#include <glad/glad.h>

#include <array>
#include <cstring>

namespace Prism
{
    using K = PrismShaderCompiler::CSL::ResourceKind;

    static RendererID s_LastComputeProgram = 0;

    static GLenum ComputeAccessToGL(bool readOnly, bool writeOnly)
    {
        if (readOnly && !writeOnly)  return GL_READ_ONLY;
        if (writeOnly && !readOnly)  return GL_WRITE_ONLY;
        return GL_READ_WRITE;
    }

    // 按声明类型分派，不按调用过的 setter 分派（见 .claude/plan-inner-compiler.md §2.5）
    static void UploadUniform(int32_t location, PrismShaderCompiler::GLSLType type, const uint8_t* data)
    {
        using T = PrismShaderCompiler::GLSLType;

        switch (type)
        {
        case T::Bool:
        case T::Int:   glUniform1i(location, *reinterpret_cast<const int32_t*>(data));   break;
        case T::UInt:  glUniform1ui(location, *reinterpret_cast<const uint32_t*>(data)); break;
        case T::Float: glUniform1f(location, *reinterpret_cast<const float*>(data));     break;

        case T::BVec2:
        case T::IVec2: glUniform2iv(location, 1, reinterpret_cast<const int32_t*>(data));   break;
        case T::BVec3:
        case T::IVec3: glUniform3iv(location, 1, reinterpret_cast<const int32_t*>(data));   break;
        case T::BVec4:
        case T::IVec4: glUniform4iv(location, 1, reinterpret_cast<const int32_t*>(data));   break;

        case T::UVec2: glUniform2uiv(location, 1, reinterpret_cast<const uint32_t*>(data)); break;
        case T::UVec3: glUniform3uiv(location, 1, reinterpret_cast<const uint32_t*>(data)); break;
        case T::UVec4: glUniform4uiv(location, 1, reinterpret_cast<const uint32_t*>(data)); break;

        case T::Vec2:  glUniform2fv(location, 1, reinterpret_cast<const float*>(data));     break;
        case T::Vec3:  glUniform3fv(location, 1, reinterpret_cast<const float*>(data));     break;
        case T::Vec4:  glUniform4fv(location, 1, reinterpret_cast<const float*>(data));     break;

        default: break;
        }
    }

    OpenGLComputeShader::OpenGLComputeShader(const std::string& filePath)
        : ComputeShader(filePath)
    {
        m_Kernels.reserve(m_Compiled.Kernels.size());
        for (size_t i = 0; i < m_Compiled.Kernels.size(); ++i)
        {
            const auto& compiled = m_Compiled.Kernels[i];

            Ref<Kernel> kernel = Ref<Kernel>::Create();
            kernel->Name = compiled.Name;
            kernel->GroupSizeX = compiled.GroupSizeX;
            kernel->GroupSizeY = compiled.GroupSizeY;
            kernel->GroupSizeZ = compiled.GroupSizeZ;
            kernel->Shader = Shader::Create(m_Compiled, (uint32_t)i).As<OpenGLShader>();
            kernel->UniformData.assign(m_Compiled.UniformBlockSize, 0);

            if (!kernel->Shader)
            {
                PR_CORE_ERROR("OpenGLComputeShader '{}': kernel '{}' 未产生 OpenGLShader", m_Name, kernel->Name);
            }
            else
            {
                const RendererID program = kernel->Shader->GetRendererID();

                kernel->UniformLocations.reserve(m_Compiled.Uniforms.size());
                for (const auto& uniform : m_Compiled.Uniforms)
                    kernel->UniformLocations.push_back(glGetUniformLocation(program, uniform.Name.c_str()));
            }

            m_Kernels.push_back(std::move(kernel));
        }
    }

    int32_t OpenGLComputeShader::FindKernel(const std::string& name) const
    {
        for (size_t i = 0; i < m_Kernels.size(); ++i)
        {
            if (m_Kernels[i]->Name == name)
                return (int32_t)i;
        }
        PR_CORE_ERROR("ComputeShader '{}': 找不到名为 '{}' 的 kernel", m_Name, name);
        return -1;
    }

    bool OpenGLComputeShader::HasKernel(const std::string& name) const
    {
        for (const Ref<Kernel>& kernel : m_Kernels)
        {
            if (kernel->Name == name)
                return true;
        }
        return false;
    }

    void OpenGLComputeShader::GetKernelThreadGroupSizes(int32_t kernel, uint32_t& x, uint32_t& y, uint32_t& z) const
    {
        if (!IsLegalKernel(kernel))
            return;
        x = m_Kernels[kernel]->GroupSizeX;
        y = m_Kernels[kernel]->GroupSizeY;
        z = m_Kernels[kernel]->GroupSizeZ;
    }

    bool OpenGLComputeShader::IsLegalKernel(int32_t kernel) const
    {
        if (kernel < 0 || kernel >= (int32_t)m_Kernels.size())
        {
            PR_CORE_ERROR("ComputeShader '{}': 不合法的 Kernel ID {}", m_Name, kernel);
            return false;
        }
        return true;
    }

    void OpenGLComputeShader::SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::UniformBuffer);
        if (slot < 0 || !ubo)
            return;

        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([binding, ubo]()
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo.As<OpenGLUniformBuffer>()->GetRendererID());
        });
    }

    void OpenGLComputeShader::SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::StorageBuffer);
        if (slot < 0 || !ssbo)
            return;

        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([binding, ssbo]()
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo.As<OpenGLShaderStorageBuffer>()->GetRendererID());
        });
    }

    void OpenGLComputeShader::SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::Sampler2D);
        if (slot < 0 || !image)
            return;

        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([binding, image]()
        {
            glBindTextureUnit(binding, image.As<OpenGLImage2D>()->GetRendererID());
        });
    }

    void OpenGLComputeShader::SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::SamplerCube);
        if (slot < 0 || !image)
            return;

        uint32_t binding = m_Slots[slot].Binding;

        Renderer::Submit([binding, image]()
        {
            glBindTextureUnit(binding, image.As<OpenGLImageCube>()->GetRendererID());
        });
    }

    void OpenGLComputeShader::SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::Image2D);
        if (slot < 0 || !image)
            return;

        uint32_t binding = m_Slots[slot].Binding;
        GLenum access = ComputeAccessToGL(m_Slots[slot].ReadOnly, m_Slots[slot].WriteOnly);

        Renderer::Submit([binding, level, access, image]()
        {
            glBindImageTexture(binding, image.As<OpenGLImage2D>()->GetRendererID(), level, GL_FALSE, 0,
                access, Utils::OpenGLImageInternalFormat(image->GetFormat()));
        });
    }

    void OpenGLComputeShader::SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, K::ImageCube);
        if (slot < 0 || !image)
            return;

        uint32_t binding = m_Slots[slot].Binding;
        GLenum access = ComputeAccessToGL(m_Slots[slot].ReadOnly, m_Slots[slot].WriteOnly);

        Renderer::Submit([binding, level, access, image]()
        {
            glBindImageTexture(binding, image.As<OpenGLImageCube>()->GetRendererID(), level, GL_TRUE, 0,
                access, Utils::OpenGLImageInternalFormat(image->GetFormat()));
        });
    }

    void OpenGLComputeShader::Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ, bool force)
    {
        if (!IsLegalKernel(kernel))
            return;

        Ref<OpenGLComputeShader> self = this;

        Renderer::Submit([self, kernel, groupsX, groupsY, groupsZ]()
        {
            Ref<Kernel> kernelInfo = self->m_Kernels[kernel];
            if (!kernelInfo->Shader)
                return;

            RendererID program = kernelInfo->Shader->GetRendererID();
            glUseProgram(program);

            if (!kernelInfo->UniformData.empty())
            {
                const std::vector<PrismShaderCompiler::CSL::ComputeUniform>& uniforms = self->m_Compiled.Uniforms;

                for (size_t i = 0; i < uniforms.size(); ++i)
                {
                    if (kernelInfo->UniformLocations[i] < 0)
                        continue;

                    UploadUniform(kernelInfo->UniformLocations[i], uniforms[i].Type,
                                  kernelInfo->UniformData.data() + uniforms[i].Offset);
                }
            }

            glDispatchCompute(groupsX, groupsY, groupsZ);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        });
    }

    // 按声明类型分派，不按调用过的 setter 分派：SetInt 也能写 uint 槽
    void OpenGLComputeShader::WriteUniform(int32_t kernel, const std::string& name,
                                           PrismShaderCompiler::GLSLType type, const void* data, uint32_t size)
    {
        if (!IsLegalKernel(kernel))
            return;

        const int32_t index = FindUniform(name);
        if (index < 0)
            return;

        const PrismShaderCompiler::CSL::ComputeUniform& uniform = m_Compiled.Uniforms[index];

        if (uniform.Type != type || uniform.Size != size)
        {
            PR_CORE_ERROR("ComputeShader '{}': uniform '{}' 声明为 {}（{} 字节），按 {}（{} 字节）写入",
                m_Name, name, PrismShaderCompiler::GLSLTypeUtil::ToString(uniform.Type), uniform.Size,
                PrismShaderCompiler::GLSLTypeUtil::ToString(type), size);
            return;
        }

        std::array<uint8_t, 16> bytes{};
        std::memcpy(bytes.data(), data, size);

        const uint32_t offset = uniform.Offset;
        Renderer::Submit([self = Ref<OpenGLComputeShader>(this), kernel, offset, bytes, size]() mutable
        {
            std::memcpy(self->m_Kernels[kernel]->UniformData.data() + offset, bytes.data(), size);
        });
    }

    void OpenGLComputeShader::SetBool(int32_t kernel, const std::string& name, bool value)
    {
        const int32_t packed = value ? 1 : 0;
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Bool, &packed, sizeof(packed));
    }

    void OpenGLComputeShader::SetInt(int32_t kernel, const std::string& name, int32_t value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Int, &value, sizeof(value));
    }

    void OpenGLComputeShader::SetUInt(int32_t kernel, const std::string& name, uint32_t value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::UInt, &value, sizeof(value));
    }

    void OpenGLComputeShader::SetFloat(int32_t kernel, const std::string& name, float value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Float, &value, sizeof(value));
    }

    void OpenGLComputeShader::SetVector2(int32_t kernel, const std::string& name, const glm::vec2& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Vec2, &value.x, 2 * sizeof(float));
    }

    void OpenGLComputeShader::SetVector3(int32_t kernel, const std::string& name, const glm::vec3& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Vec3, &value.x, 3 * sizeof(float));
    }

    void OpenGLComputeShader::SetVector4(int32_t kernel, const std::string& name, const glm::vec4& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::Vec4, &value.x, 4 * sizeof(float));
    }

    void OpenGLComputeShader::SetIntVector2(int32_t kernel, const std::string& name, const glm::ivec2& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::IVec2, &value.x, 2 * sizeof(int32_t));
    }

    void OpenGLComputeShader::SetIntVector3(int32_t kernel, const std::string& name, const glm::ivec3& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::IVec3, &value.x, 3 * sizeof(int32_t));
    }

    void OpenGLComputeShader::SetIntVector4(int32_t kernel, const std::string& name, const glm::ivec4& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::IVec4, &value.x, 4 * sizeof(int32_t));
    }

    void OpenGLComputeShader::SetUIntVector2(int32_t kernel, const std::string& name, const glm::uvec2& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::UVec2, &value.x, 2 * sizeof(uint32_t));
    }

    void OpenGLComputeShader::SetUIntVector3(int32_t kernel, const std::string& name, const glm::uvec3& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::UVec3, &value.x, 3 * sizeof(uint32_t));
    }

    void OpenGLComputeShader::SetUIntVector4(int32_t kernel, const std::string& name, const glm::uvec4& value)
    {
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::UVec4, &value.x, 4 * sizeof(uint32_t));
    }

    void OpenGLComputeShader::SetBoolVector2(int32_t kernel, const std::string& name, const glm::bvec2& value)
    {
        const int32_t packed[2] = { value.x, value.y };
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::BVec2, packed, sizeof(packed));
    }

    void OpenGLComputeShader::SetBoolVector3(int32_t kernel, const std::string& name, const glm::bvec3& value)
    {
        const int32_t packed[3] = { value.x, value.y, value.z };
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::BVec3, packed, sizeof(packed));
    }

    void OpenGLComputeShader::SetBoolVector4(int32_t kernel, const std::string& name, const glm::bvec4& value)
    {
        const int32_t packed[4] = { value.x, value.y, value.z, value.w };
        WriteUniform(kernel, name, PrismShaderCompiler::GLSLType::BVec4, packed, sizeof(packed));
    }
}
