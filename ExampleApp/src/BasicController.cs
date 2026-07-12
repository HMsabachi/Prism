using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    public class BasicController : Behaviour
    {
        public float Speed;
        public float DistanceFromPlayer = 20.0F;

        private Entity m_PlayerEntity;
        private TransformComponent? m_Transform;
        private TransformComponent? m_PlayerTransform;

        public void OnCreate()
        {
            m_PlayerEntity = Entity.FindEntityByTag("Player");
            m_Transform = GetComponent<TransformComponent>();
            m_PlayerTransform = m_PlayerEntity.GetComponent<TransformComponent>();
        }

        public void OnUpdate()
        {
            float ts = Time.DeltaTime;
            if (m_Transform == null || m_PlayerTransform == null) return;
            Vector3 playerTranslation = m_PlayerTransform.Position;
            Vector3 Position = m_Transform.Position;
            Position.XY = playerTranslation.XY;
            Position.Z = playerTranslation.Z + DistanceFromPlayer;
            Position.Y = Math.Max(Position.Y, 2.0f);
            m_Transform.Position = Position;
        }


    }
}
