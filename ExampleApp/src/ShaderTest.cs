using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    public class ShaderTest : Behaviour
    {
        List<Material> materials = new List<Material>();
        UInt32[] data = new UInt32[128];
        public ComputeShader ComputeShader;
        public PrismShader Shader;
        public MeshRendererComponent rendererComponent;


        private readonly float[] m_Input = new float[100];
        private ComputeShader m_SquareShader;
        private ShaderStorageBuffer m_Buffer;
        private ShaderStorageBufferReadback? m_Request;
        private List<ShaderStorageBufferReadback> m_Readbacks = new List<ShaderStorageBufferReadback>();
        private uint m_Frame;
        private bool m_Dispatched;

        public void OnCreate()
        {
            // Test: PrismShader
            rendererComponent = GetComponent<MeshRendererComponent>();
            PrismShader shader;
            ComputeShader computeShader;
            if (Shader != null)
            {
                shader = Shader;
            }
            else
            {
                shader = PrismShader.GetShader("Standard/PrismPBR");
                Log.Warn("Shader is not valid, using default shader");
            }
            if (this.ComputeShader != null)
            {
                computeShader = this.ComputeShader;
            }
            else
            {
                computeShader = ComputeShader.Create("Assets/Shaders/Environment.ComputeShader");
                Log.Warn("ComputeShader is not valid, using default compute shader");
            }

            Log.Trace($"Shader Name: {shader.Name}");
            Log.Trace($"Shader FilePath: {shader.FilePath}");
            Log.Trace($"Shader FileName: {shader.FileName}");
            Log.Trace($"Shader Extension: {shader.Extension}");
            Log.Trace($"Shader UnifromCount: {shader.GetUniformCount()}");
            for (UInt32 i = 0; i < shader.GetUniformCount(); i++)
            {
                string uniformName = shader.GetUniformName(i);
                string uniformDisplayName = shader.GetUniformDisplayName(i);
                UniformType uniformType = shader.GetUniformType(i);
                Vector3 uniformValue = shader.GetUniformDefaultValue<Vector3>(i);
                Log.Trace($"Uniform {i}: {uniformName}, {uniformDisplayName}, Type: {uniformType}, Value: {uniformValue}");
            }
            ImageSpecification specification = new ImageSpecification { Format = ImageFormat.RGB8, Width = 8, Height = 8 };
            Image2D image = Image2D.Create<UInt32>(specification, data);
            Log.Trace($"Image2D: {image.Width}x{image.Height}, Format: {image.Format}, Samples: {image.Samples}");
            Texture2D texture = Texture2D.Create(10, 10);
            Log.Trace($"Texture2D: {texture.Width}x{texture.Height}, Format: {texture.Format}");
            image = texture.GetImage();
            Log.Trace($"Texture2D Image2D: {image.Width}x{image.Height}, Format: {image.Format}, Samples: {image.Samples}");
            Log.Trace($"ComputeShader: {computeShader.Name}");
        }

        public void OnUpdate()
        {
        }

        public void OnFixedUpdate()
        {
            if (Input.IsKeyPressed(KeyCode.R))
            {
                Material material = rendererComponent.Material;

                material.SetTexture2D("u_AlbedoTexture", Texture2D.Create(10, 10));
                material.SetKeyword("ALBEDO_MAP", true);
            }
            if (Input.IsKeyPressed(KeyCode.T))
            {
                Material material = rendererComponent.Material;
                material.SetKeyword("ALBEDO_MAP", false);
            }
        }

 
    }
}
