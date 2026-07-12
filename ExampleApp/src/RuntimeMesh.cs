using Prism;
using System;

namespace Example
{
    public class RuntimeMesh : Behaviour
    {
        public Mesh? mesh;

        private MeshRendererComponent m_MeshRenderer;
        private RandomColor m_RandomColor;
        private Mesh? m_BoundMesh;

        private void OnCreate()
        {
            m_MeshRenderer = GetComponent<MeshRendererComponent>();
            m_RandomColor = CreateComponent<RandomColor>();
            if (mesh != null) m_MeshRenderer.Mesh = mesh;
            m_BoundMesh = mesh;
        }

        private void OnUpdate()
        {
            if (m_BoundMesh != mesh && mesh != null)
            {
                m_MeshRenderer.Mesh = mesh;
                m_BoundMesh = mesh;
            }
        }
    }
}
