using Prism;

namespace Example
{
    public class FPSPlayer : Behaviour
    {
        public float WalkingSpeed = 10.0F;
        public float RunSpeed = 20.0F;
        public float JumpForce = 50.0F;

        private float m_CurrentSpeed;

        private RigidBodyComponent m_RigidBody;
        private TransformComponent m_Transform;

        private Entity m_CameraEntity;

        private int m_CollisionCounter = 0;
        private bool Colliding => m_CollisionCounter > 0;

        void OnCreate()
        {
            m_Transform = GetComponent<TransformComponent>();
            m_RigidBody = GetComponent<RigidBodyComponent>();

            m_CurrentSpeed = WalkingSpeed;

            m_CameraEntity = Entity.FindEntityByTag("Camera");
        }

        void OnFixedUpdate()
        {
            m_CurrentSpeed = Input.IsKeyPressed(KeyCode.LeftControl) ? RunSpeed : WalkingSpeed;

            if (Input.IsKeyPressed(KeyCode.W))
                m_RigidBody.AddForce(m_Transform.Forward * m_CurrentSpeed);
            else if (Input.IsKeyPressed(KeyCode.S))
                m_RigidBody.AddForce(m_Transform.Forward * -m_CurrentSpeed);

            if (Input.IsKeyPressed(KeyCode.A))
                m_RigidBody.AddForce(m_Transform.Right * -m_CurrentSpeed);
            else if (Input.IsKeyPressed(KeyCode.D))
                m_RigidBody.AddForce(m_Transform.Right * m_CurrentSpeed);

            if (Input.IsKeyPressed(KeyCode.Space) && Colliding)
                m_RigidBody.AddForce(Vector3.Up * JumpForce);
        }

        void OnUpdate()
        {
            if (m_CameraEntity == null)
                return;

            // TODO: This workflow needs to be improved. (Will be fixed by object parenting)
            Matrix4 cameraTransform = m_CameraEntity.GetTransform();
            Vector3 cameraTranslation = cameraTransform.Translation;
            Vector3 translation = Entity.GetTransform().Translation;
            cameraTranslation.XZ = translation.XZ;
            cameraTranslation.Y = translation.Y + 1.5F;
            cameraTransform.Translation = cameraTranslation;
            m_CameraEntity.SetTransform(cameraTransform);
        }

        public void OnCollisionBegin(float data)
        {
            m_CollisionCounter++;
        }

        public void OnCollisionEnd(float data)
        {
            m_CollisionCounter--;
        }
    }
}
