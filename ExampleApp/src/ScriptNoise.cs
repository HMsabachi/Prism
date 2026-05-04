using Prism;

namespace Example
{
    public class ScriptNoise : Entity
    {
        private void OnCreate()
        {
            var material = new Material("Custom/NoiseTest");
            var materialInstance = new MaterialInstance(material);
            var meshComponent = GetComponent<MeshComponent>();
            meshComponent.SetMaterial(0, materialInstance);
            Log.Trace("创建 MateruakComponent");
        }
        private void OnUpdate()
        {

        }
    }
}
