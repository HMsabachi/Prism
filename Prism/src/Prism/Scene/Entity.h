#pragma once
#include "Prism/Renderer/Mesh.h"

namespace Prism 
{

	class PRISM_API Entity
	{
	public:
		~Entity();

		// TODO: 这里应该在Component中实现
		void SetMesh(const Ref<Mesh>& mesh) { m_Mesh = mesh; }
		Ref<Mesh> GetMesh() { return m_Mesh; }

		void SetMaterial(const Ref<MaterialInstance>& material) { m_Material = material; }
		Ref<MaterialInstance> GetMaterial() { return m_Material; }

		const glm::mat4& GetTransform() const { return m_Transform; }
		glm::mat4& Transform() { return m_Transform; }
	
		const std::string& GetName() const { return m_Name; }
	private:
		Entity(const std::string& name);
	private:
		glm::mat4 m_Transform;
		std::string m_Name;

		// TODO: 临时
		Ref<Mesh> m_Mesh;
		Ref<MaterialInstance> m_Material;

		friend class Scene;
	};

}