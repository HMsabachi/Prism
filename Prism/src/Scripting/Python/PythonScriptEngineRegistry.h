#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace Prism
{
    class Entity;

    extern std::unordered_map<uint64_t, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
    extern std::unordered_map<uint64_t, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;

    class PythonScriptEngineRegistry
    {
    public:
        static void RegisterAll();
    };
}
