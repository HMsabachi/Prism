#pragma once

namespace Prism
{
	class CSharpScriptEngine;

	class ScriptEngineRegistry
	{
	public:
		static void RegisterAll(CSharpScriptEngine& engine);
	};
}