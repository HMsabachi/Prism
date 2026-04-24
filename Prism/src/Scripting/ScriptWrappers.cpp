#include "prpch.h"
#include "ScriptWrappers.h"
#include "Native/String.h"

#include "Prism/Core/Input.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include <glm/gtc/type_ptr.hpp>

namespace Prism {
	extern std::unordered_map<uint32_t, Scene*> s_ActiveScenes;
}

namespace Prism {
	namespace Script
	{
#pragma region Legacy Functions

		void Prism_Log_Core_Trace(const char* mes)
		{
			PR_CORE_TRACE("[Script]: {0}", mes);
		}
		void Prism_Log_Core_Info(const char* mes)
		{
			PR_CORE_INFO("[Script]: {0}", mes);
		}
		void Prism_Log_Core_Warn(const char* mes)
		{
			PR_CORE_WARN("[Script]: {0}", mes);
		}
		void Prism_Log_Core_Error(const char* mes)
		{
			PR_CORE_ERROR("[Script]: {0}", mes);
		}
		void Prism_Log_Core_Fatal(const char* mes)
		{
			PR_CORE_FATAL("[Script]: {0}", mes);
		}
#pragma endregion

#pragma region Log

		void Prism_Log_LogMessage(LogLevel level, Native::String inFormattedMessage)
		{
			std::string message = NativeStringToCString(&inFormattedMessage);
			message = "[Script]: " + message;
			switch (level)
			{
			case LogLevel::Trace:
				PR_CORE_TRACE(message);
				break;
			case LogLevel::Debug:
				PR_CORE_INFO(message);
				break;
			case LogLevel::Info:
				PR_CORE_INFO(message);
				break;
			case LogLevel::Warn:
				PR_CORE_WARN(message);
				break;
			case LogLevel::Error:
				PR_CORE_ERROR(message);
				break;
			case LogLevel::Critical:
				PR_CORE_FATAL(message);
				break;
			}
			Native::FreeNativeString(&inFormattedMessage);
		}

#pragma endregion

#pragma region Native
		Prism::Native::String Prism_Native_CreateNativeString(const char* cstr){ return Native::CreateNativeString(cstr); }
		const char* Prism_Native_NativeStringToCString(const Native::String* str){ return Native::NativeStringToCString(str); }
		void Prism_Native_FreeNativeString(Native::String* str){ return Native::FreeNativeString(str); }
		Prism::Native::String Prism_Native_CopyNativeString(const Native::String* src){ return Native::CopyNativeString(src); }
#pragma endregion


#pragma region Time
		float Prism_Time_GetDeltaTime(){ return Time::GetDeltaTime(); }
		float Prism_Time_GetUnscaledDeltaTime(){ return Time::GetUnscaledDeltaTime(); }
		float Prism_Time_GetTime(){ return Time::GetTime(); }
		float Prism_Time_GetUnscaledTime(){ return Time::GetUnscaledTime(); }
		float Prism_Time_GetFixedDeltaTime(){ return Time::GetFixedDeltaTime(); }
		int64_t Prism_Time_GetFrameCount(){ return (int64_t)Time::GetFrameCount(); }
		void Prism_Time_SetTimeScale(float scale){ Time::SetTimeScale(scale);}
		float Prism_Time_GetTimeScale(){ return Time::GetTimeScale();}
#pragma endregion

#pragma region Math
		float Prism_Noise_PerlinNoise(float x, float y)
		{
			// TODO: Implement Perlin Noise
			return x * y;
		}
#pragma endregion 

#pragma region Input
		bool Prism_Input_IsKeyPressed(KeyCode key)
		{
			return Input::IsKeyPressed(key);
		}
#pragma endregion 

#pragma region Entity		
		enum class ComponentID
		{
			None = 0,
			Transform = 1,
			Mesh = 2,
			Script = 3,
			SpriteRenderer = 4
		};
		void Prism_Entity_GetTransform(uint32_t sceneID, uint32_t entityID, glm::mat4* outTransform)
		{
			PR_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			memcpy(outTransform, glm::value_ptr(transformComponent.Transform), sizeof(glm::mat4));
		}

		void Prism_Entity_SetTransform(uint32_t sceneID, uint32_t entityID, glm::mat4* inTransform)
		{
			PR_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			memcpy(glm::value_ptr(transformComponent.Transform), inTransform, sizeof(glm::mat4));
		}
#pragma endregion 
	}
}