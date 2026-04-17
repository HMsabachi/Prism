#pragma once
#include "Prism/Renderer/Mesh.h"

namespace Prism 
{

	class PRISM_API Entity
	{
	public:
		Entity();
		~Entity();

		// TODO: 这里应该在Component中实现
		void SetMesh(const Ref<Mesh>& mesh) { m_Mesh = mesh; }
		Ref<Mesh> GetMesh() { return m_Mesh; }

		void SetMaterial(const Ref<MaterialInstance>& material) { m_Material = material; }
		Ref<MaterialInstance> GetMaterial() { return m_Material; }

		const glm::mat4& GetTransform() const { return m_Transform; }
		glm::mat4& Transform() { return m_Transform; }
	private:
		glm::mat4 m_Transform;

		// TODO: 临时
		Ref<Mesh> m_Mesh;
		Ref<MaterialInstance> m_Material;
	};

}