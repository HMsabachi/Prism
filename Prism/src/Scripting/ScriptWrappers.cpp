#include "prpch.h"
#include "ScriptWrappers.h"
#include "Prism/Core/Math/Noise.h"

#include "Prism/Core/Input.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include <glm/gtc/type_ptr.hpp>

#include <Rolky/String.hpp>
#include <Rolky/Type.hpp>
#include <Rolky/Array.hpp>

namespace Prism {
	extern std::unordered_map<uint32_t, Scene*> s_ActiveScenes;
	extern std::unordered_map<Rolky::TypeId, std::function<void(Entity&)>> s_CreateComponentFuncs;
	extern std::unordered_map<Rolky::TypeId, std::function<bool(Entity&)>> s_HasComponentFuncs;
}

namespace Prism {
	namespace Script
	{


#pragma region Log

		void Prism_Log_LogMessage(LogLevel level, Rolky::String inFormattedMessage)
		{
			std::string message = inFormattedMessage;
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
			Rolky::String::Free(inFormattedMessage);
		}

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
			return Noise::PerlinNoise(x, y);
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

		void Prism_Entity_CreateComponent(uint32_t sceneID, uint32_t entityID, Rolky::ReflectionType type)
		{
			PR_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");
			Rolky::Type mType = type;
			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			s_CreateComponentFuncs[mType.GetTypeId()](entity);
		}

		bool Prism_Entity_HasComponent(uint32_t sceneID, uint32_t entityID, Rolky::ReflectionType type)
		{
			PR_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");
			Rolky::Type mType = type;
			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			bool result = s_HasComponentFuncs[mType.GetTypeId()](entity);
			return result;
		}


#pragma endregion 

#pragma region Mesh
		void* Prism_MeshComponent_GetMesh(uint32_t sceneID, uint32_t entityID)
		{
			PR_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			auto& meshComponent = entity.GetComponent<MeshComponent>();
			return new Ref<Mesh>(meshComponent.Mesh);
		}

		void Prism_MeshComponent_SetMesh(uint32_t sceneID, uint32_t entityID, Ref<Mesh>* inMesh)
		{
			PR_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			auto& meshComponent = entity.GetComponent<MeshComponent>();
			meshComponent.Mesh = inMesh ? *inMesh : nullptr;
		}

		Prism::Ref<Prism::Mesh>* Prism_Mesh_Constructor(Rolky::String filepath)
		{
			std::string path = filepath;
			Rolky::String::Free(filepath);
            return new Ref<Mesh>(new Mesh(path));
		}

		void Prism_Mesh_Destructor(Ref<Mesh>* _this)
		{
			Ref<Mesh>* instance = (Ref<Mesh>*)_this;
			delete _this;
		}

		Prism::Ref<Prism::Material>* Prism_Mesh_GetMaterial(Ref<Mesh>* inMesh)
		{
			Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
			return new Ref<Material>(mesh->GetMaterial());
		}

		Prism::Ref<Prism::MaterialInstance>* Prism_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int32_t index)
		{
			Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
			const auto& materials = mesh->GetMaterials();

			PR_CORE_ASSERT(index < materials.size());
			return new Ref<MaterialInstance>(materials[index]);
		}

		int32_t Prism_Mesh_GetMaterialCount(Ref<Mesh>* inMesh)
		{
			Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
			const auto& materials = mesh->GetMaterials();
			return materials.size();
		}

		void* Prism_MeshFactory_CreatePlane(float width, float height)
		{
			return new Ref<Mesh>(new Mesh("assets/models/Plane1m.obj"));
		}

#pragma endregion

#pragma region Texture2D
		void* Prism_Texture2D_Constructor(uint32_t width, uint32_t height)
		{
			auto result = Texture2D::Create(TextureFormat::RGBA, width, height);
            return new Ref<Texture2D>(result);
		}

		void Prism_Texture2D_Destructor(Ref<Texture2D>* _this)
		{
			delete _this;
		}

		void Prism_Texture2D_SetData(Ref<Texture2D>* _this, Rolky::Array<glm::vec4> inData, int32_t count)
		{
			Ref<Texture2D>& instance = *_this;
			uint32_t dataSize = count * sizeof(glm::vec4) / 4;
			instance->Lock();
			Buffer buffer = instance->GetWriteableBuffer();
			PR_CORE_ASSERT(dataSize <= buffer.Size);
			uint8_t* pixels = (uint8_t*)buffer.Data;
			uint32_t index = 0;
			for (int i = 0; i < instance->GetWidth() * instance->GetHeight(); i++)
			{
				glm::vec4& value = inData[i];
				*pixels++ = (uint32_t)(value.x * 255.0f);
				*pixels++ = (uint32_t)(value.y * 255.0f);
				*pixels++ = (uint32_t)(value.z * 255.0f);
				*pixels++ = (uint32_t)(value.w * 255.0f);
			}
			inData.Free(inData);
			instance->Unlock();
		}

#pragma endregion

#pragma region Material
		void Prism_Material_Destructor(Ref<Material>* _this)
		{
			delete _this;
		}


		void Prism_Material_SetFloat(Ref<Material>* _this, Rolky::String uniform, float value)
		{
			Ref<Material>& instance = *(Ref<Material>*)_this;
			instance->Set(uniform, value);
			uniform.Free(uniform);
		}

		void Prism_Material_SetTexture(Ref<Material>* _this, Rolky::String uniform, Ref<Texture2D>* texture)
		{
			Ref<Material>& instance = *(Ref<Material>*)_this;
			instance->Set(uniform, *texture);
			uniform.Free(uniform);
		}

		void Prism_MaterialInstance_Destructor(Ref<MaterialInstance>* _this)
		{
			delete _this;
		}

		void Prism_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, Rolky::String uniform, float value)
		{
			Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
			instance->Set(uniform, value);
			uniform.Free(uniform);
		}

		void Prism_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, Rolky::String uniform, glm::vec3* value)
		{
			Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
			instance->Set(uniform, *value);
			uniform.Free(uniform);
		}

		void Prism_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, Rolky::String uniform, Ref<Texture2D>* texture)
		{
			Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
			instance->Set(uniform, *texture);
			uniform.Free(uniform);
		}

#pragma endregion


	}
}