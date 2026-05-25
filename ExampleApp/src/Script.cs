
using Prism;
using System;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Example
{
    public class Script : Behaviour
    {
        public float Speed = 5.0f;
        float time = 0.0f;

        public void OnCreate()
        {
            Log.Trace("Script.OnCreate");
        }

        public void OnUpdate()
        {
            if (!Input.IsKeyPressed(KeyCode.Space)) return;
            time += Time.DeltaTime;
            if (time > 100.0f) time = 0.0f;
            Matrix4 transform = Entity.GetTransform();
            Vector3 translation = transform.Translation;
            translation.X = MathF.Sin(time) * Speed;
            transform.Translation = translation;
            Entity.SetTransform(transform);
            //Log.Trace("transform.position: {0}", HasComponent<TransformComponent>());
        }

    }
}
