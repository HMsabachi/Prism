#include "prpch.h"
#include "Shader.h"

#include "Prism/Core/Hash.h"
#include "Prism/ShaderCompiler/ShaderCompiler.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Prism/Renderer/Shader/PrismShaderCache.h"

namespace Prism
{
    inline static uint64_t GenerateShaderKeyHash(const PrismShaderCompiler::CompiledShader& shader,
        uint32_t passIndex,
        const std::vector<std::string>& keywords)
    {
        constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        constexpr uint64_t OFFSET_BASIS = 14695981039346656037ULL;
        uint64_t hash = OFFSET_BASIS;
        hash ^= Hash::GenerateFNVHash64(shader.ShaderName); hash *= FNV_PRIME;
        hash ^= static_cast<uint64_t>(passIndex); hash *= FNV_PRIME;
        for (const auto& kw : keywords) { hash ^= Hash::GenerateFNVHash64(kw); hash *= FNV_PRIME; }
        hash ^= static_cast<uint64_t>(static_cast<uint8_t>(RendererAPI::Current())); hash *= FNV_PRIME;
        return hash;
    }

    inline static uint64_t GenerateShaderKeyHash(const PrismShaderCompiler::CompiledComputeShader& shader,
        uint32_t kernelIndex)
    {
        constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        constexpr uint64_t OFFSET_BASIS = 14695981039346656037ULL;
        uint64_t hash = OFFSET_BASIS;
        hash ^= Hash::GenerateFNVHash64(shader.ShaderName); hash *= FNV_PRIME;
        hash ^= static_cast<uint64_t>(kernelIndex); hash *= FNV_PRIME;
        hash ^= static_cast<uint64_t>(static_cast<uint8_t>(RendererAPI::Current())); hash *= FNV_PRIME;
        return hash;
    }

    Ref<Shader> Shader::Create(const PrismShaderCompiler::CompiledShader& shader,
        uint32_t passIndex,
        const std::vector<std::string>& keywords)
    {
        auto& compiler = ShaderCompiler::Get();
        auto& cache = PrismShaderCache::Get();
        ShaderCacheEntry entry;
        uint64_t keyHash = GenerateShaderKeyHash(shader, passIndex, keywords);
        cache.FindEntry(keyHash, &entry);
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
        {
            if (entry.KeyHash == 0)
            {
                auto out = compiler.GenerateGLSL(shader, passIndex, keywords);
                entry.KeyHash = keyHash; entry.Backend = static_cast<uint64_t>(RendererAPIType::OpenGL);
                entry.Source_1 = { (const uint8_t*)out.VertexShader.c_str(), out.VertexShader.size() };
                entry.Source_2 = { (const uint8_t*)out.FragmentShader.c_str(), out.FragmentShader.size() };
                cache.AddEntry(entry);
                cache.FindEntry(keyHash, &entry);
            }
            return Ref<Shader>(new OpenGLShader(entry.Source_1, entry.Source_2));
        }
        case RendererAPIType::Vulkan:
        {
            if (entry.KeyHash == 0)
            {
                auto out = compiler.GenerateSPIRV(shader, passIndex, keywords);
                entry.KeyHash = keyHash; entry.Backend = static_cast<uint64_t>(RendererAPIType::Vulkan);
                entry.Source_1 = { (const uint8_t*)out.SpirvVertex.data(), out.SpirvVertex.size() * sizeof(uint32_t) };
                entry.Source_2 = { (const uint8_t*)out.SpirvFragment.data(), out.SpirvFragment.size() * sizeof(uint32_t) };
                entry.Reflection = out.Reflection.Descriptors;
                cache.AddEntry(entry);
                cache.FindEntry(keyHash, &entry);
            }
            return Ref<Shader>(new VulkanShader(entry.Source_1, entry.Source_2, entry.Reflection));
        }
        default:
            PR_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
        }
    }

    Ref<Shader> Shader::Create(const PrismShaderCompiler::CompiledComputeShader& shader,
        uint32_t kernelIndex)
    {
        auto& compiler = ShaderCompiler::Get();
        auto& cache = PrismShaderCache::Get();
        ShaderCacheEntry entry;
        uint64_t keyHash = GenerateShaderKeyHash(shader, kernelIndex);
        cache.FindEntry(keyHash, &entry);
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
        {
            if (entry.KeyHash == 0)
            {
                auto out = compiler.GenerateComputeGLSL(shader, kernelIndex);
                entry.KeyHash = keyHash; entry.Backend = static_cast<uint64_t>(RendererAPIType::OpenGL);
                entry.Source_1 = { (const uint8_t*)out.Source.c_str(), out.Source.size() };
                entry.Reflection = out.Reflection.Descriptors;
                cache.AddEntry(entry);
                cache.FindEntry(keyHash, &entry);
            }
            return Ref<Shader>(new OpenGLShader(entry.Source_1));
            break;
        }
        case RendererAPIType::Vulkan:
        {
            if (entry.KeyHash == 0)
            {
                auto out = compiler.GenerateComputeSPIRV(shader, kernelIndex);
                entry.KeyHash = keyHash; entry.Backend = static_cast<uint64_t>(RendererAPIType::Vulkan);
                entry.Source_1 = { (const uint8_t*)out.Spirv.data(), out.Spirv.size() * sizeof(uint32_t) };
                entry.Reflection = out.Reflection.Descriptors;
                cache.AddEntry(entry);
                cache.FindEntry(keyHash, &entry);
            }
            return Ref<Shader>(new VulkanShader(entry.Source_1, entry.Reflection));
            break;
        }
        default:
            PR_CORE_ASSERT(false, "Unknown RendererAPI!");
        }
        return nullptr;
    }
   
}
