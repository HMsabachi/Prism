using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Prism;
namespace Example
{
    public class MapGenerator : Behaviour
    {
        // [EditorSlider("MapWidth Custom Name", 2, 0, 1024)]
        public int MapWidth = 128;
        public int MapHeight = 128;
        public int Octaves = 4;
        public float Persistance = 0.74f;
        public int Seed = 21;
        public float Lacunarity = 3.0f;
        public Vector2 Offset = new Vector2(13.4f, 6.26f);
        public float NoiseScale = 0.5f;

        public float Speed = 0.0f;
        public void GenerateMap()
        {
            //float[,] noiseMap = Noise.GenerateNoiseMap(mapWidth, mapHeight, noiseScale);
            float[,] noiseMap = Noise.GenerateNoiseMap(MapWidth, MapHeight, Seed, NoiseScale, Octaves, Persistance, Lacunarity, Offset);

            uint width = (uint)noiseMap.GetLength(0);
            uint height = (uint)noiseMap.GetLength(1);

            Texture2D texture = new Texture2D(width, height);
            Vector4[] colorMap = new Vector4[width * height];
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    colorMap[x + y * width] = Vector4.Lerp(Color.Black, Color.White, noiseMap[x, y]);
                }
            }

            texture.SetData(colorMap);

            Log.Trace("HasComponent - TransformComponent = {0}", HasComponent<TransformComponent>());
            Log.Trace("HasComponent - ScriptComponent = {0}", HasComponent<ScriptComponent>());
            Log.Trace("HasComponent - MeshComponent = {0}", HasComponent<MeshComponent>());

            MeshComponent meshComponent = GetComponent<MeshComponent>();
            if (meshComponent == null)
            {
                Log.Trace("MeshComponent is null!");
                meshComponent = CreateComponent<MeshComponent>();
            }
            meshComponent.Mesh = MeshFactory.CreatePlane(1.0f, 1.0f);

            Log.Trace("Mesh has {0} materials!", meshComponent.Mesh.GetMaterialCount());

            MaterialInstance material = meshComponent.Mesh.GetMaterial(1);
            material.SetKeyword("ALBEDO_MAP", true);
            material.Set("u_AlbedoTexture", texture);

            TransformComponent transformComponent = GetComponent<TransformComponent>();
            Vector3 position = Entity.GetTransform().Translation;
            transformComponent.Transform = Matrix4.Scale(new Vector3(10f, 1.0f, 10f)) * Matrix4.Translate(position);
        }

        private void OnCreate()
        {
            GenerateMap();
        }

        private void OnUpdate()
        {
            float ts = Time.DeltaTime;
            Matrix4 transform = Entity.GetTransform();
            Vector3 translation = transform.Translation;
            translation.Y += ts * Speed;
            if (Input.IsKeyPressed(KeyCode.Space))
            {
                translation.Y -= 10.0f;
            }
            transform.Translation = translation;
            Entity.SetTransform(transform);
        }
    }
}
