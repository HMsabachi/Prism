using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    class Sink : Behaviour
    {

        public float SinkSpeed;

        void OnCreate()
        {

        }

        void OnUpdate()
        {
            float ts = Time.DeltaTime;
            Matrix4 transform = Entity.GetTransform();
            Vector3 translation = transform.Translation;

            translation.Y -= SinkSpeed * ts;

            transform.Translation = translation;
            Entity.SetTransform(transform);
        }

    }
}