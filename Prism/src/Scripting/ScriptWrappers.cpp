#include "prpch.h"
#include "ScriptWrappers.h"
#include "Native/String.h"

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

		void Log_LogMessage(LogLevel level, Native::String inFormattedMessage)
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

		enum class ComponentID
		{
			None = 0,
			Transform = 1,
			Mesh = 2,
			Script = 3,
			SpriteRenderer = 4
		};



		////////////////////////////////////////////////////////////////
		// Math ////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////

		/*float Prism_Noise_PerlinNoise(float x, float y)
		{
			return Noise::PerlinNoise(x, y);
		}*/

		////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////
		// Input ///////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////

		/*bool Prism_Input_IsKeyPressed(KeyCode key)
		{
			return Input::IsKeyPressed(key);
		}*/

		////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////
		// Entity //////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////

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
	}
}