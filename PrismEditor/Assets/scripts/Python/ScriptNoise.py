from PrismEngine import Behaviour, Log, MeshRendererComponent, Material, PrismShader


class ScriptNoise(Behaviour):
    def OnCreate(self):
        shader = PrismShader.GetShader("Custom/NoiseTest")
        material = Material(shader)
        meshComponent = self.GetComponent(MeshRendererComponent)
        meshComponent.SetMaterial(0, material)

    def OnUpdate(self):
        pass
