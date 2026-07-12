#pragma once

#include <PrismShaderCore/Compiler.h>

namespace Prism
{

class ShaderCompiler
{
public:
    static void Init();
    static PrismShaderCompiler::ShaderCompiler& Get();

private:
    static inline PrismShaderCompiler::ShaderCompiler* s_Instance = nullptr;
};

} // namespace Prism
