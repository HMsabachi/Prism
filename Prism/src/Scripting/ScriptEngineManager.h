#pragma once
#include "ScriptEngine.h"
#include <memory>

namespace Prism
{
	class PRISM_API ScriptEngineManager
	{
	public:
		static void Init();
		static void Shutdown();

		static ScriptEngine* Get();
		static void Register(std::unique_ptr<ScriptEngine> engine);

	private:
		static std::unique_ptr<ScriptEngine> s_Engine;
	};
}