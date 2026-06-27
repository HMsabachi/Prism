using System;
using Prism;

namespace Example
{
    public class FPSPlayer : Behaviour
    {
        public float WalkingSpeed = 10.0F;
        public float RunSpeed = 20.0F;
        public float JumpForce = 50.0F;
        public float CameraForwardOffset = 0.2F;
        public float CameraYOffset = 0.85F;

        public float MouseSensitivity = 10.0F;

        private float m_CurrentSpeed;
        private float m_CurrentYMovement = 0.0F;

        private Vector2 m_MovementDirection = new Vector2(0.0F);
        private bool m_ShouldJump = false;

        private RigidBodyComponent m_RigidBody;
        private TransformComponent m_Transform;
        private TransformComponent m_CameraTransform;

        private Entity m_CameraEntity;

        private Vector2 m_LastMousePosition;

        private int m_CollisionCounter = 0;
        private bool Colliding => m_CollisionCounter > 0;

        private Collider[] m_Colliders = new Collider[10];

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

            if (Input.IsKeyPressed(KeyCode.Escape) && Input.GetCursorMode() == CursorMode.Locked)
                Input.SetCursorMode(CursorMode.Normal);

            //if (Input.IsMouseButtonPressed(MouseButton.Left) && Input.GetCursorMode() == CursorMode.Normal)
            //    Input.SetCursorMode(CursorMode.Locked);

            m_CurrentSpeed = Input.IsKeyPressed(KeyCode.LeftControl) ? RunSpeed : WalkingSpeed;

            UpdateRaycasting();
            UpdateMovementInput();
            UpdateRotation();
            UpdateCameraTransform();
        }

        void OnFixedUpdate()
        {
            UpdateMovement();
        }

        private void UpdateRotation()
        {
            float ts = Time.DeltaTime;
            Vector2 currentMousePosition = Input.GetMousePosition();
            Vector2 delta = m_LastMousePosition - currentMousePosition;
            m_CurrentYMovement = delta.X * MouseSensitivity * ts;
            float xRotation = delta.Y * MouseSensitivity * ts;

            if (delta.Y != 0.0F || delta.X != 0.0F)
            {
                m_CameraTransform.Rotation += new Vector3(xRotation, m_CurrentYMovement, 0.0F);
            }

            m_CameraTransform.Rotation = new Vector3(Mathf.Clamp(m_CameraTransform.Rotation.X, -80.0F, 80.0F), m_CameraTransform.Rotation.YZ);

            m_LastMousePosition = currentMousePosition;
            m_Transform.Rotation = new Vector3(0.0f, m_CameraTransform.Rotation.Y, 0.0f);
        }

        private void UpdateMovementInput()
        {
            if (Input.IsKeyPressed(KeyCode.W))
                m_MovementDirection.Y = 1.0F;
            else if (Input.IsKeyPressed(KeyCode.S))
                m_MovementDirection.Y = -1.0F;
            else
                m_MovementDirection.Y = 0.0F;

            if (Input.IsKeyPressed(KeyCode.A))
                m_MovementDirection.X = -1.0F;
            else if (Input.IsKeyPressed(KeyCode.D))
                m_MovementDirection.X = 1.0F;
            else
                m_MovementDirection.X = 0.0F;

            m_ShouldJump = Input.IsKeyPressed(KeyCode.Space) && !m_ShouldJump;
        }

        private void UpdateRaycasting()
        {
            RaycastHit hitInfo;
            if (Input.IsKeyPressed(KeyCode.H) && Physics.Raycast(m_CameraTransform.Position + (m_CameraTransform.Transform.Forward * 5.0F), m_CameraTransform.Transform.Forward, 20.0F, out hitInfo))
            {
                var entity = Entity.FindEntityByID(hitInfo.EntityID);
                var renderer = entity?.GetComponent<MeshRendererComponent>();
                renderer?.Material.SetFloat("u_Metalness", 1.0f);
            }
            if (Input.IsKeyPressed(KeyCode.G) && Physics.Raycast(m_CameraTransform.Position + (m_CameraTransform.Transform.Forward * 5.0F), m_CameraTransform.Transform.Forward, 20.0F, out hitInfo))
            {
                var entity = Entity.FindEntityByID(hitInfo.EntityID);
                var renderer = entity?.GetComponent<MeshRendererComponent>();
                renderer?.Material.SetFloat("u_Metalness", 0.0f);
            }

            if (Input.IsKeyPressed(KeyCode.L))
            {
                int numColliders = Physics.OverlapBoxNonAlloc(m_Transform.Position, new Vector3(1.0F), m_Colliders);
                Log.Trace(numColliders);

                for (int i = 0; i < numColliders; i++)
                {
                    Collider c = m_Colliders[i];
                    Log.Trace(c);
                }
            }
        }

        private void UpdateMovement()
        {
            m_RigidBody.Rotate(new Vector3(0.0F, m_CurrentYMovement, 0.0F));

            Vector3 movement = m_CameraTransform.Transform.Right * m_MovementDirection.X + m_CameraTransform.Transform.Forward * m_MovementDirection.Y;
            movement.Normalize();
            Vector3 velocity = movement * m_CurrentSpeed;
            velocity.Y = m_RigidBody.GetLinearVelocity().Y;
            m_RigidBody.SetLinearVelocity(velocity);

            if (m_ShouldJump && Colliding)
            {
                m_RigidBody.AddForce(Vector3.Up * JumpForce, ForceMode.VelocityChange);
                m_ShouldJump = false;
            }
        }

        private void UpdateCameraTransform()
        {
            Vector3 position = m_Transform.Position + m_Transform.Transform.Forward * CameraForwardOffset;
            position.Y = m_Transform.Position.Y + CameraYOffset;
            m_CameraTransform.Position = position;
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
