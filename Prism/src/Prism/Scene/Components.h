#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Prism/Core/UUID.h"

#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Scene/SceneCamera.h"
#include "glm/gtx/quaternion.hpp"

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
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::quat Rotation = { 1.0f, 0.0f, 0.0f, 0.0f }; // identity quaternion
		glm::vec3 Scale    = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;

		explicit TransformComponent(const glm::mat4& transform)
		{
			SetTransform(transform);
		}

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), Position)
				 * glm::toMat4(Rotation)
				 * glm::scale(glm::mat4(1.0f), Scale);
		}

		void SetTransform(const glm::mat4& mat)
		{
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(mat, Scale, Rotation, Position, skew, perspective);
		}
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

	struct RigidBody2DComponent
	{
		enum class Type { Static, Dynamic, Kinematic };
		Type BodyType = Type::Static;
		bool FixedRotation = false;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		RigidBody2DComponent() = default;
		RigidBody2DComponent(const RigidBody2DComponent& other)
			: BodyType(other.BodyType), FixedRotation(other.FixedRotation), RuntimeBody(nullptr) {
		}
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 1.0f, 1.0f };

		float Density = 1.0f;
		float Friction = 1.0f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent& other)
			: Offset(other.Offset), Size(other.Size), Density(other.Density), Friction(other.Friction), RuntimeFixture(nullptr) {
		}
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 1.0f;

		float Density = 1.0f;
		float Friction = 1.0f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent& other)
			: Offset(other.Offset), Radius(other.Radius), Density(other.Density), Friction(other.Friction), RuntimeFixture(nullptr) {
		}
	};

	struct SphereColliderComponent
	{
		float Radius = 0.5f;

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent& other) = default;
	};

}