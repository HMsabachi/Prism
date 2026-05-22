using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    class PlayerSphere : Entity
    {
        public float HorizontalForce = 10.0f;
        public float JumpForce = 10.0f;

        private RigidBodyComponent m_PhysicsBody;
        private MaterialInstance m_MeshMaterial;

        int m_CollisionCounter = 0;

        public Vector3 MaxSpeed = new Vector3();

        private bool Colliding => m_CollisionCounter > 0;

        private TransformComponent m_Transform;

        void OnCreate()
        {
            m_PhysicsBody = GetComponent<RigidBodyComponent>();
            m_Transform = GetComponent<TransformComponent>();

            MeshComponent meshComponent = GetComponent<MeshComponent>();
            m_MeshMaterial = meshComponent.Mesh.GetMaterial(0);
            m_MeshMaterial.Set("u_Metalness", 0.0f);

            AddCollisionBeginCallback(OnPlayerCollisionBegin);
            AddCollisionEndCallback(OnPlayerCollisionEnd);
        }

        void OnPlayerCollisionBegin(float value)
        {
            Log.Trace("PlayerSphere OnPlayerCollisionBegin {0}", value);
            m_CollisionCounter++;
        }

        void OnPlayerCollisionEnd(float value)
        {
            m_CollisionCounter--;
        }

        void OnFixedUpdate()
        {
            float movementForce = HorizontalForce;

            if (!Colliding)
            {
                movementForce *= 0.4f;
            }
            Vector3 forward = m_Transform.Forward;
            Vector3 right = m_Transform.Right;
            Vector3 up = m_Transform.Up;
            if (Input.IsKeyPressed(KeyCode.W))
                m_PhysicsBody.AddForce(forward * movementForce);
            else if (Input.IsKeyPressed(KeyCode.S))
                m_PhysicsBody.AddForce(forward * -movementForce);
            if (Input.IsKeyPressed(KeyCode.D))
                m_PhysicsBody.AddForce(right * movementForce);
            else if (Input.IsKeyPressed(KeyCode.A))
                m_PhysicsBody.AddForce(right * -movementForce);

            if (Colliding && Input.IsKeyPressed(KeyCode.Space))
                m_PhysicsBody.AddForce(up * JumpForce);
            if (Colliding)
                m_MeshMaterial.Set("u_AlbedoColor", new Vector3(1.0f, 0.0f, 0.0f));
            else
                m_MeshMaterial.Set("u_AlbedoColor", new Vector3(0.8f, 0.8f, 0.8f));
            Vector3 linearVelocity = m_PhysicsBody.GetLinearVelocity();
            linearVelocity.Clamp(new Vector3(-MaxSpeed.X, -1000, -MaxSpeed.Z), MaxSpeed);
            m_PhysicsBody.SetLinearVelocity(linearVelocity);

            if (Input.IsKeyPressed(KeyCode.R))
            {
                Matrix4 transform = GetTransform();
                transform.Translation = new Vector3(0.0f);
                SetTransform(transform);
            }

        }

    }
}