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
        public void OnCreate()
        {
            // Test: PrismShader
            var shader = PrismShader.GetShader("Standard/PrismPBR");
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
            for (UInt32 i = 0; i < 100; i++)
            {
                materials.Add(new Material(shader));
            }
        }

        public void OnUpdate()
        {

        }

        public void OnFixedUpdate()
        {
        }

 
    }
}
