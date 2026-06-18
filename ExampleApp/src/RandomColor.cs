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
        Random random = new Random();
        Material material;
        private void OnCreate()
        {
            MeshRendererComponent meshComponent = GetComponent<MeshRendererComponent>();
            material = meshComponent.GetMaterial(0);
            GenerateColor();
        }
        private void OnUpdate()
        {

        }
        public void GenerateColor()
        {
            if (!Enabled) return;
            float r = (float)random.NextDouble();
            float g = (float)random.NextDouble();
            float b = (float)random.NextDouble();
            material.Set("u_AlbedoColor", new Vector3(r, g, b));
        }
    }
}