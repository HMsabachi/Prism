from Prism import Behaviour, Log, MeshComponent
from Prism.Renderer.Material import Material, MaterialInstance


class ScriptNoise(Behaviour):
    def OnCreate(self):
        material = Material("Custom/NoiseTest")
        materialInstance = MaterialInstance(material)
        meshComponent = self.GetComponent(MeshComponent)
        meshComponent.SetMaterial(0, materialInstance)
        Log.Trace("创建 MaterialInstance")

    def OnUpdate(self):
        pass
