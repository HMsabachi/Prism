#pragma once

namespace Prism
{
    struct ShaderCommand;
}

namespace Prism
{

    class OpenGLStateCache
    {
    public:
        static void Init();

        static void Apply(const ShaderCommand& newCommand);

        static void Reset();

    private:
        static void ApplyBlendIfChanged(const ShaderCommand& cmd);
        static void ApplyCullIfChanged(const ShaderCommand& cmd);
        static void ApplyDepthIfChanged(const ShaderCommand& cmd);
        static void ApplyZWriteIfChanged(const ShaderCommand& cmd);
    };
}