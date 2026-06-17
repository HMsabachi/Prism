using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    class PlayerCube : Behaviour
    {
        public float HorizontalForce = 10.0f;
        public float JumpForce = 10.0f;

        public Vector2 Velocity = new Vector2();
        private RigidBody2DComponent m_PhysicsBody;
        private Material m_MeshMaterial;

        private int m_CollisionCounter = 0;

        public Vector2 MaxSpeed = new Vector2();

        private bool Colliding => m_CollisionCounter > 0;

        void OnCreate()
        {
            m_PhysicsBody = GetComponent<RigidBody2DComponent>();

            MeshComponent meshComponent = GetComponent<MeshComponent>();
            m_MeshMaterial = meshComponent.Mesh.GetMaterial(0);
            m_MeshMaterial.Set("u_Metalness", 0.0f);
        }

        public void OnCollisionBegin(float data)
        {
            m_CollisionCounter++;
        }

        public void OnCollisionEnd(float data)
        {
            m_CollisionCounter--;
        }

        void OnFixedUpdate()
        {
            float ts = Time.FixedDeltaTime;
            float movementForce = HorizontalForce;
            Velocity = m_PhysicsBody.LinearVelocity;
            if (!Colliding)
            {
                movementForce *= 0.4f;
            }

            if (Input.IsKeyPressed(KeyCode.D))
                m_PhysicsBody.ApplyLinearImpulse(new Vector2(movementForce, 0), new Vector2(), true);
            else if (Input.IsKeyPressed(KeyCode.A))
                m_PhysicsBody.ApplyLinearImpulse(new Vector2(-movementForce, 0), new Vector2(), true);

            if (Colliding && Input.IsKeyPressed(KeyCode.Space))
                m_PhysicsBody.ApplyLinearImpulse(new Vector2(0, JumpForce), new Vector2(0, 0), true);

            if (m_CollisionCounter > 0)
                m_MeshMaterial.Set("u_AlbedoColor", new Vector3(1.0f, 0.0f, 0.0f));
            else
                m_MeshMaterial.Set("u_AlbedoColor", new Vector3(0.8f, 0.8f, 0.8f));

        }

    }
}
