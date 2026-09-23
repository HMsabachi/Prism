using Prism;
using System;
using System.Collections.Generic;
using System.Linq;
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


        private readonly float[] m_Input = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
        private ComputeShader m_SquareShader;
        private ShaderStorageBuffer m_Buffer;
        private ShaderStorageBufferReadback? m_Request;
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

            // 平方测试
            m_SquareShader = ComputeShader.Create("Assets/Shaders/Test.ComputeShader");
            m_Buffer = ShaderStorageBuffer.Create((uint)(sizeof(float) * m_Input.Length), BufferUsage.Dynamic);
            m_Buffer.SetData(m_Input);
            Log.Info($"SquareTest: input  = [{string.Join(", ", m_Input)}], buffer = {m_Buffer.Size} bytes");

        }

        public void OnUpdate()
        {
            if (!m_Dispatched)
            {
                int kernel = m_SquareShader.FindKernel("CSSquare");
                m_SquareShader.SetBuffer(kernel, "u_Data", m_Buffer);
                m_SquareShader.Dispatch(kernel, (uint)((m_Input.Length + 63) / 64), 1, 1);
                m_Request = m_Buffer.RequestReadback();
                m_Dispatched = true;
                return;
            }
            if (m_Request == null) return;
            m_Frame++;
            Log.Info($"SquareTest: frame {m_Frame} dispatched, IsDone = {m_Request.IsDone}, size = {m_Request.Size}");
            if (!m_Request.IsDone)
            {
                Log.Info($"SquareTest: frame {m_Frame} waiting for readback...");
                return;
            }
            float[] result = new float[m_Input.Length];
            m_Request.GetData(result);
            Log.Info($"SquareTest: output = [{string.Join(", ", result)}] at frame {m_Frame}");
            m_Request = null;
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
