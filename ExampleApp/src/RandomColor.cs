using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    class RandomColor : Behaviour
    {
        public float Timer = 0;
        private Random random = new Random();
        private Material material;
        private void OnCreate()
        {
            MeshRendererComponent meshComponent = GetComponent<MeshRendererComponent>();
            material = meshComponent.Material;
            GenerateColor();
            Timer = 0;
        }
        private void OnUpdate()
        {
            Timer += Time.DeltaTime;
            if (Timer > 1.5 )
            {
                GenerateColor();
                Timer = 0;
            }
        }
        public void GenerateColor()
        {
            if (!Enabled) return;
            float r = (float)random.NextDouble();
            float g = (float)random.NextDouble();
            float b = (float)random.NextDouble();
            material.SetVector3("u_AlbedoColor", new Vector3(r, g, b));
        }
    }
}
