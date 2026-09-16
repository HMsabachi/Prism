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

    namespace
    {
        struct GLDispatchBinding
        {
            uint32_t Binding = 0;
            K Kind = K::StorageBuffer;
            bool ReadOnly = false;
            bool WriteOnly = false;
            uint32_t Level = 0;
            Ref<RefCounted> Resource;
        };
    }

    OpenGLComputeShader::OpenGLComputeShader(const std::string& filePath)
        : ComputeShader(filePath)
    {
        m_KernelValues.resize(m_Kernels.size());
        for (auto& values : m_KernelValues)
            values.Slots.resize(m_Slots.size());
    }

    void OpenGLComputeShader::SetSlot(int32_t kernel, const std::string& name, K kind,
        Ref<RefCounted> resource, uint32_t level)
    {
        if (!IsLegalKernel(kernel))
            return;

        int32_t slot = FindSlot(name, kind);
        if (slot < 0)
            return;

        SlotValue& value = m_KernelValues[kernel].Slots[slot];
        value.Resource = std::move(resource);
        value.Level = level;
    }

    void OpenGLComputeShader::SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo)
    {
        SetSlot(kernel, name, K::UniformBuffer, ubo, 0);
    }

    void OpenGLComputeShader::SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo)
    {
        SetSlot(kernel, name, K::StorageBuffer, ssbo, 0);
    }

    void OpenGLComputeShader::SetTexture2D(int32_t kernel, const std::string& name, Ref<Image2D> image)
    {
        SetSlot(kernel, name, K::Sampler2D, image, 0);
    }

    void OpenGLComputeShader::SetTextureCube(int32_t kernel, const std::string& name, Ref<ImageCube> image)
    {
        SetSlot(kernel, name, K::SamplerCube, image, 0);
    }

    void OpenGLComputeShader::SetImage2D(int32_t kernel, const std::string& name, Ref<Image2D> image, uint32_t level)
    {
        SetSlot(kernel, name, K::Image2D, image, level);
    }

    void OpenGLComputeShader::SetImageCube(int32_t kernel, const std::string& name, Ref<ImageCube> image, uint32_t level)
    {
        SetSlot(kernel, name, K::ImageCube, image, level);
    }

    void OpenGLComputeShader::Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
    {
        if (!IsLegalKernel(kernel))
            return;

        Ref<Shader> kernelShader = m_KernelShaders[kernel];
        if (!kernelShader)
            return;

        const std::vector<SlotValue>& values = m_KernelValues[kernel].Slots;
        std::vector<GLDispatchBinding> captured;
        captured.reserve(m_Slots.size());
        for (size_t i = 0; i < m_Slots.size(); ++i)
        {
            if (!values[i].Resource)
                continue;

            captured.push_back({ m_Slots[i].Binding, m_Slots[i].Kind, m_Slots[i].ReadOnly,
                m_Slots[i].WriteOnly, values[i].Level, values[i].Resource });
        }

        RendererID program = kernelShader.As<OpenGLShader>()->GetRendererID();

        Renderer::Submit([captured = std::move(captured), program, groupsX, groupsY, groupsZ]()
        {
            if (s_LastComputeProgram != program)
            {
                glUseProgram(program);
                s_LastComputeProgram = program;
            }

            for (const GLDispatchBinding& entry : captured)
            {
                if (entry.Kind == K::UniformBuffer)
                {
                    Ref<UniformBuffer> ubo = entry.Resource.As<UniformBuffer>();
                    glBindBufferBase(GL_UNIFORM_BUFFER, entry.Binding, ubo.As<OpenGLUniformBuffer>()->GetRendererID());
                }
                else if (entry.Kind == K::StorageBuffer)
                {
                    Ref<ShaderStorageBuffer> ssbo = entry.Resource.As<ShaderStorageBuffer>();
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, entry.Binding, ssbo.As<OpenGLShaderStorageBuffer>()->GetRendererID());
                }
                else
                {
                    Ref<Image> image = entry.Resource.As<Image>();
                    bool isCube = entry.Kind == K::SamplerCube || entry.Kind == K::ImageCube;
                    RendererID imageId = isCube
                        ? image.As<OpenGLImageCube>()->GetRendererID()
                        : image.As<OpenGLImage2D>()->GetRendererID();

                    if (entry.Kind == K::Image2D || entry.Kind == K::Image3D || entry.Kind == K::ImageCube)
                    {
                        glBindImageTexture(entry.Binding, imageId, entry.Level, entry.Kind != K::Image2D, 0,
                            ComputeAccessToGL(entry.ReadOnly, entry.WriteOnly),
                            Utils::OpenGLImageInternalFormat(image->GetFormat()));
                    }
                    else
                    {
                        glBindTextureUnit(entry.Binding, imageId);
                    }
                }
            }

            glDispatchCompute(groupsX, groupsY, groupsZ);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        });
    }
}
