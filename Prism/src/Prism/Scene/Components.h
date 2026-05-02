#pragma once

#include <glm/glm.hpp>
#include "Prism/Core/UUID.h"

#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Scene/SceneCamera.h"

namespace Prism {

	struct IDComponent
	{
		UUID ID = 0;
	};

	struct TagComponent
	{
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent& other)
			: Tag(other.Tag) {
		}
		TagComponent(const std::string& tag)
			: Tag(tag) {
		}
		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct TransformComponent
	{
		glm::mat4 Transform;

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other)
			: Transform(other.Transform) {
		}
		TransformComponent(const glm::mat4& transform)
			: Transform(transform) {
		}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	struct MeshComponent
	{
		Ref<Prism::Mesh> Mesh;
		MeshComponent() = default;
		MeshComponent(const MeshComponent& other)
			: Mesh(other.Mesh) {
		}
		MeshComponent(const Ref<Prism::Mesh>& mesh)
			: Mesh(mesh) {
		}

		operator Ref<Prism::Mesh>() { return Mesh; }
	};

	struct ScriptComponent
	{
		std::string ModuleName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other)
			: ModuleName(other.ModuleName) {
		}
		ScriptComponent(const std::string& moduleName)
			: ModuleName(moduleName) {
		}
	};

	struct CameraComponent
	{
		//OrthographicCamera Camera;
		SceneCamera Camera;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other)
			: Camera(other.Camera), Primary(other.Primary) {
		}

		operator SceneCamera& () { return Camera; }
		operator const SceneCamera& () const { return Camera; }
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent& other)
			: Color(other.Color), Texture(other.Texture), TilingFactor(other.TilingFactor) {
		}
	};

	struct MaterialComponent
	{
		Ref<MaterialInstance> Material;

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent& other)
			: Material(other.Material) {
		}
		MaterialComponent(const Ref<MaterialInstance>& material)
			: Material(material) {
		}

		operator Ref<Prism::MaterialInstance>() { return Material; }
	};

}