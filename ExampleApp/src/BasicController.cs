using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    public class BasicController : Entity
    {
        public float Speed;

        private Entity m_PlayerEntity;

        public void OnCreate()
        {
            m_PlayerEntity = FindEntityByTag("Player");
        }

        public void OnUpdate()
        {
            float ts = Time.DeltaTime;
            Matrix4 transform = GetTransform();
            Vector3 translation = transform.Translation;
            translation.XY = m_PlayerEntity.GetTransform().Translation.XY;
            translation.Y = Math.Max(translation.Y, 4.5f);
            transform.Translation = translation;
            SetTransform(transform);
        }


    }
}
