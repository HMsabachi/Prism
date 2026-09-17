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
        public ComputeShader computeShader;
        public PrismShader shader;
        public void OnCreate()
        {
            // Test: PrismShader
            PrismShader shader;
            ComputeShader computeShader;
            if (this.shader != null)
            {
                shader = this.shader;
            }
            else
            {
                shader = PrismShader.GetShader("Standard/PrismPBR");
                Log.Warn("Shader is not valid, using default shader");
            }
            if (this.computeShader != null)
            {
                computeShader = this.computeShader;
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
            Image2D image = Image2D.Create<UInt32>(ImageFormat.RGB8, 8, 8, data, 1);
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
        }

 
    }
}
