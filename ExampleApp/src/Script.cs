
using Prism;
using System;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Example
{
    public class Script : Entity
    {
        public float Speed = 5.0f;
        float time = 0.0f;

        public void OnCreate()
        {
            Log.Trace("Script.OnCreate");
        }

        public void OnUpdate()
        {
            time += 0.01f;
            Matrix4 transform = GetTransform();
            Vector3 translation = transform.Translation;
            translation.X = MathF.Sin(time) * Speed;
            transform.Translation = translation;
            SetTransform(transform);
            //Log.Trace("transform.position: {0}", transform.Translation);
        }

    }
}