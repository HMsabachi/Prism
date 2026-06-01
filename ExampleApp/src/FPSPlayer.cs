using Prism;

namespace Example
{
    public class FPSPlayer : Behaviour
    {
        public float WalkingSpeed = 10.0F;
        public float RunSpeed = 20.0F;
        public float JumpForce = 50.0F;
        public float MouseSensitivity = 10.0F;

        private float m_CurrentSpeed;

        private RigidBodyComponent m_RigidBody;
        private TransformComponent m_Transform;
        private TransformComponent m_CameraTransform;

        private Entity m_CameraEntity;

        private Vector2 m_LastMousePosition;
        private float m_CameraRotationX = 0.0F;

        private int m_CollisionCounter = 0;
        private bool Colliding => m_CollisionCounter > 0;

        void OnCreate()
        {
            m_Transform = GetComponent<TransformComponent>();
            m_RigidBody = GetComponent<RigidBodyComponent>();

            m_CurrentSpeed = WalkingSpeed;

            m_CameraEntity = Entity.FindEntityByTag("Camera");
            m_CameraTransform = m_CameraEntity.GetComponent<TransformComponent>();

            m_LastMousePosition = Input.GetMousePosition();

            Input.SetCursorMode(CursorMode.Locked);
        }

        void OnUpdate()
        {
            float ts = Time.DeltaTime;

            if (Input.IsKeyPressed(KeyCode.Escape) && Input.GetCursorMode() == CursorMode.Locked)
            {
                Input.SetCursorMode(CursorMode.Normal);
            }

            m_CurrentSpeed = Input.IsKeyPressed(KeyCode.LeftControl) ? RunSpeed : WalkingSpeed;

            UpdateRotation(ts);
            UpdateMovement();
            UpdateCameraTransform();
        }

        private void UpdateRotation(float ts)
        {
            Vector2 currentMousePosition = Input.GetMousePosition();
            Vector2 delta = m_LastMousePosition - currentMousePosition;
            m_RigidBody.Rotate(new Vector3(0.0F, delta.X * MouseSensitivity, 0.0F) * ts);

            if (delta.Y != 0.0F)
            {
                m_CameraRotationX += delta.Y * MouseSensitivity * ts;
                //m_CameraRotationX = Mathf.Clamp(m_CameraRotationX, -80.0F, 80.0F);
                m_CameraTransform.Rotation = new Vector3(m_CameraRotationX, 0.0F, 0.0F);
            }

            m_LastMousePosition = currentMousePosition;
        }

        private void UpdateMovement()
        {
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

        private void UpdateCameraTransform()
        {
            Matrix4 cameraTransform = m_CameraTransform.Transform;
            Vector3 cameraTranslation = cameraTransform.Translation;
            Vector3 translation = m_Transform.Transform.Translation;
            cameraTranslation.XZ = translation.XZ;
            cameraTranslation.Y = translation.Y + 1.5F;
            m_CameraTransform.Transform = m_Transform.Transform;

            Vector3 cameraRotation = m_Transform.Rotation;
            cameraRotation.XZ = m_CameraTransform.Rotation.XZ;
            m_CameraTransform.Rotation = cameraRotation;
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
