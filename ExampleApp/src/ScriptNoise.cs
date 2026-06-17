using Prism;

namespace Example
{
    public class ScriptNoise : Behaviour
    {
        private void OnCreate()
        {
            var material = new Material("Custom/NoiseTest");
            var meshComponent = GetComponent<MeshComponent>();
            meshComponent.SetMaterial(0, material);
        }
        private void OnUpdate()
        {

        }
    }
}
