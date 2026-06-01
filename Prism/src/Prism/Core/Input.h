#pragma once
#include "Core.h"
#include "KeyCodes.h"

namespace Prism
{

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	class PRISM_API Input
	{
	public:
		inline static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

		inline static void SetCursorMode(CursorMode mode) { s_Instance->SetCursorModeImpl(mode); }
		inline static CursorMode GetCursorMode() { return s_Instance->GetCursorModeImpl(); }
	protected:
		virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;

		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

		virtual void SetCursorModeImpl(CursorMode mode) = 0;
		virtual CursorMode GetCursorModeImpl() = 0;
	private:
		static Input* s_Instance;
	};

}
