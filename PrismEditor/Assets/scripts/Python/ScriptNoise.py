from Prism import Behaviour, Log, MeshComponent
from Prism.Renderer.Material import Material


class ScriptNoise(Behaviour):
    def OnCreate(self):
        material = Material("Custom/NoiseTest")
        meshComponent = self.GetComponent(MeshComponent)
        meshComponent.SetMaterial(0, material)

    def OnUpdate(self):
        pass
