#include "prpch.h"
#include "ScriptEngineManager.h"

namespace Prism
{
	std::unique_ptr<ScriptEngine> ScriptEngineManager::s_Engine = nullptr;

	void ScriptEngineManager::Init()
	{
		s_Engine = nullptr;
	}

	void ScriptEngineManager::Shutdown()
	{
		if (s_Engine)
		{
			s_Engine->Shutdown();
			s_Engine.reset();
		}
	}

	ScriptEngine* ScriptEngineManager::Get()
	{
		return s_Engine.get();
	}

	void ScriptEngineManager::Register(std::unique_ptr<ScriptEngine> engine)
	{
		s_Engine = std::move(engine);
	}
}