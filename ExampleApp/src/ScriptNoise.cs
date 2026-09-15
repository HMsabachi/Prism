using Prism;

namespace Example
{
    public class ScriptNoise : Behaviour
    {
        private void OnCreate()
        {
            var shader = PrismShader.GetShader("Custom/NoiseTest");
            var material = Material.Create(shader);
            var meshComponent = GetComponent<MeshRendererComponent>();
            meshComponent.SetMaterial(0, material);
        }
        private void OnUpdate()
        {

        }
    }
}
