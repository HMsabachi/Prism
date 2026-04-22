#pragma once

#include <glm/glm.hpp>

#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Camera/Camera.h"

namespace Prism {

	struct TagComponent
	{
		std::string Tag;

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct TransformComponent
	{
		glm::mat4 Transform;

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	struct MeshComponent
	{
		Ref<Prism::Mesh> Mesh;

		operator Ref<Prism::Mesh>() { return Mesh; }
	};

	struct ScriptComponent
	{
		// TODO: C# script
		std::string ModuleName;
	};

	struct CameraComponent
	{
		//OrthographicCamera Camera;
		Prism::Camera Camera;
		bool Primary = true;

		operator Prism::Camera& () { return Camera; }
		operator const Prism::Camera& () const { return Camera; }
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;
	};


}