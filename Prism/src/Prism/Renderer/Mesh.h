#pragma once
#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Prism/Renderer/Buffer.h"

namespace Prism {

	class PRISM_API Mesh
	{
	public:
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec3 Binormal;
			glm::vec2 Texcoord;
		};
		static_assert(sizeof(Vertex) == 14 * sizeof(float));
		static const int NumAttributes = 5;

		struct Index
		{
			uint32_t V1, V2, V3;
		};
		static_assert(sizeof(Index) == 3 * sizeof(uint32_t));

		Mesh(const std::string& filename);
		~Mesh();

		void Render();

		inline const std::string& GetFilePath() const { return m_FilePath; }
	private:
		std::vector<Vertex> m_Vertices;
		std::vector<Index> m_Indices;

		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;

		std::string m_FilePath;
	};
}