using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    class PlayerCube : Entity
    {
        public float HorizontalForce = 10.0f;
        public float VerticalForce = 10.0f;

        private RigidBody2DComponent m_PhysicsBody;
        private MaterialInstance m_MeshMaterial;

        void OnCreate()
        {
            m_PhysicsBody = GetComponent<RigidBody2DComponent>();

            MeshComponent meshComponent = GetComponent<MeshComponent>();
            m_MeshMaterial = meshComponent.Mesh.GetMaterial(0);
            m_MeshMaterial.Set("u_Metalness", 0.0f);
        }
        void OnUpdate()
        {
            float ts = Time.DeltaTime;
            if (Input.IsKeyPressed(KeyCode.D))
                m_PhysicsBody.ApplyLinearImpulse(new Vector2(HorizontalForce, 0), new Vector2(), true);
            else if (Input.IsKeyPressed(KeyCode.A))
                m_PhysicsBody.ApplyLinearImpulse(new Vector2(-HorizontalForce, 0), new Vector2(), true);

            if (Input.IsKeyPressed(KeyCode.Space))
                m_PhysicsBody.ApplyLinearImpulse(new Vector2(0, VerticalForce), new Vector2(0, -10), true);

            Vector3 color = new Vector3(0.8f, 0.8f, 0.8f);
            if (Input.IsKeyPressed(KeyCode.Q))
                color = new Vector3(0.0f, 1.0f, 0.0f);

            m_MeshMaterial.Set("u_AlbedoColor", color);

            if (Input.IsKeyPressed(KeyCode.R))
            {
                Matrix4 transform = GetTransform();
                transform.Translation = new Vector3(0.0f);
                SetTransform(transform);
            }

        }

    }
}