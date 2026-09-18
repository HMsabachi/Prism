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

            if (!kernel->Shader)
                PR_CORE_ERROR("OpenGLComputeShader '{}': kernel '{}' 未产生 OpenGLShader", m_Name, kernel->Name);

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

        Ref<Kernel> kernelInfo = m_Kernels[kernel];
        if (!kernelInfo->Shader)
            return;

        RendererID program = kernelInfo->Shader->GetRendererID();

        Renderer::Submit([program, groupsX, groupsY, groupsZ]()
        {
            if (s_LastComputeProgram != program)
            {
                glUseProgram(program);
                s_LastComputeProgram = program;
            }

            glDispatchCompute(groupsX, groupsY, groupsZ);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        });
    }
}
