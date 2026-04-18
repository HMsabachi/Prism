#pragma once


struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Prism
{
	class Scene;
	class Mesh;
}

namespace Prism
{
	class PRISM_API SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();
	private:
		void DrawMeshNode(const Ref<Mesh>& mesh);
		void MeshNodeHierarchy(const Ref<Mesh>& mesh, aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
	private:
		Ref<Scene> m_Context;
		Ref<Mesh> m_SelectionContext;
	};
}
