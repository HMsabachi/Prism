from Prism import Behaviour, Log, MeshRendererComponent
from Prism.Renderer.Material import Material
from Prism.Renderer.PrismShader import PrismShader


class ScriptNoise(Behaviour):
    def OnCreate(self):
        shader = PrismShader.GetShader("Custom/NoiseTest")
        material = Material(shader)
        meshComponent = self.GetComponent(MeshRendererComponent)
        meshComponent.SetMaterial(0, material)

    def OnUpdate(self):
        pass
