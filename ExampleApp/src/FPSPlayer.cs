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
        private float m_RotationY = 0.0F;

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

            m_RotationY = m_Transform.Rotation.Y;

            Input.SetCursorMode(CursorMode.Locked);
        }

        void OnUpdate()
        {

            if (Input.IsKeyPressed(KeyCode.Escape) && Input.GetCursorMode() == CursorMode.Locked)
                Input.SetCursorMode(CursorMode.Normal);

            //if (Input.IsMouseButtonPressed(MouseButton.Left) && Input.GetCursorMode() == CursorMode.Normal)
            //    Input.SetCursorMode(CursorMode.Locked);

            m_CurrentSpeed = Input.IsKeyPressed(KeyCode.LeftControl) ? RunSpeed : WalkingSpeed;

            UpdateRotation();
            UpdateMovement();
            UpdateCameraTransform();
        }

        private void UpdateRotation()
        {
            float ts = Time.DeltaTime;
            Vector2 currentMousePosition = Input.GetMousePosition();
            Vector2 delta = m_LastMousePosition - currentMousePosition;
            m_RotationY += delta.X * MouseSensitivity * ts;
            //m_Transform.Rotation = new Vector3(0.0F, m_RotationY, 0.0F);

            if (delta.Y != 0.0F)
            {
                m_CameraRotationX += delta.Y * MouseSensitivity * ts;
                m_CameraRotationX = Mathf.Clamp(m_CameraRotationX, -80.0F, 80.0F);
            }

            m_LastMousePosition = currentMousePosition;
        }

        private void UpdateMovement()
        {
            RaycastHit hitInfo;
            if (Input.IsKeyPressed(KeyCode.H) && Physics.Raycast(m_CameraTransform.Transform.Translation + (m_CameraTransform.Forward * 5.0F), m_CameraTransform.Forward, 20.0F, out hitInfo))
            {
                var entity = Entity.FindEntityByID(hitInfo.EntityID);
                var mesh = entity?.GetComponent<MeshComponent>()?.Mesh;
                mesh?.GetMaterial(0)?.Set("u_Metalness", 1.0f);
            }
            if (Input.IsKeyPressed(KeyCode.G) && Physics.Raycast(m_CameraTransform.Transform.Translation + (m_CameraTransform.Forward * 5.0F), m_CameraTransform.Forward, 20.0F, out hitInfo))
            {
                var entity = Entity.FindEntityByID(hitInfo.EntityID);
                var mesh = entity?.GetComponent<MeshComponent>()?.Mesh;
                mesh?.GetMaterial(0)?.Set("u_Metalness", 0.0f);
            }

            if (Input.IsKeyPressed(KeyCode.L))
            {
                Collider[] colliders = Physics.OverlapSphere(m_Transform.Transform.Translation, 1.0F);
                Log.Trace(colliders.Length);

                foreach (Collider c in colliders)
                {
                    Log.Trace("EntityID: {0}", c.EntityID);
                    Log.Trace("IsTrigger: {0}", c.IsTrigger);
                    Log.Trace("IsBox: {0}", c.Is<BoxCollider>());
                    Log.Trace("IsSphere: {0}", c.Is<SphereCollider>());
                    Log.Trace("IsCapsule: {0}", c.Is<CapsuleCollider>());
                }
            }

            if (Input.IsKeyPressed(KeyCode.W))
                m_RigidBody.AddForce(m_CameraTransform.Forward * m_CurrentSpeed);
            else if (Input.IsKeyPressed(KeyCode.S))
                m_RigidBody.AddForce(m_CameraTransform.Forward * -m_CurrentSpeed);

            if (Input.IsKeyPressed(KeyCode.A))
                m_RigidBody.AddForce(m_CameraTransform.Right * -m_CurrentSpeed);
            else if (Input.IsKeyPressed(KeyCode.D))
                m_RigidBody.AddForce(m_CameraTransform.Right * m_CurrentSpeed);

            if (Input.IsKeyPressed(KeyCode.Space) && Colliding)
                m_RigidBody.AddForce(Vector3.Up * JumpForce);
            var linearVelocity = m_RigidBody.LinearVelocity;
            linearVelocity.Clamp(new Vector3(-m_CurrentSpeed, -m_CurrentSpeed, -m_CurrentSpeed), new Vector3(m_CurrentSpeed, m_CurrentSpeed, m_CurrentSpeed));
            m_RigidBody.LinearVelocity = linearVelocity;
        }

        private void UpdateCameraTransform()
        {
            Vector3 cameraTranslation = m_CameraTransform.Position;
            Vector3 translation = m_Transform.Transform.Translation;
            cameraTranslation.XZ = translation.XZ;
            cameraTranslation.Y = translation.Y + 1.5F;
            m_CameraTransform.Position = cameraTranslation;

            m_CameraTransform.Rotation = new Vector3(m_CameraRotationX, m_RotationY, 0.0F);
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
