using Prism;

namespace Example
{
    public class ScriptNoise : Entity
    {
        private void OnCreate()
        {
            var material = new Material("Custom/NoiseTest");
            var materialInstance = new MaterialInstance(material);
            var materialComponent = CreateComponent<MaterialComponent>();
            materialComponent.Material = materialInstance;
            Log.Trace("创建 MateruakComponent");
        }
        private void OnUpdate()
        {

        }
    }
}
