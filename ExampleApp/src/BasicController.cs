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

        public void OnCreate()
        {
            m_PlayerEntity = Entity.FindEntityByTag("Player");
        }

        public void OnUpdate()
        {
            float ts = Time.DeltaTime;
            /*Matrix4 transform = Entity.GetTransform();

            Vector3 playerTranslation = m_PlayerEntity.GetTransform().Translation;
            Vector3 translation = transform.Translation;
            translation.XY = playerTranslation.XY;
            translation.Z = playerTranslation.Z + DistanceFromPlayer;
            translation.Y = Math.Max(translation.Y, 2.0f);
            transform.Translation = translation;
            Entity.SetTransform(transform);*/
        }


    }
}
