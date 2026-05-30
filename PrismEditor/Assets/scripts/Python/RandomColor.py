import random
from Prism import Behaviour, MeshComponent
from Prism.Math import Vector3
from Prism.Renderer.Material import MaterialInstance


class RandomColor(Behaviour):
    def OnCreate(self):
        self.random = random.Random()
        meshComponent = self.GetComponent(MeshComponent)
        self.material = meshComponent.Mesh.GetMaterial(0)
        self.GenerateColor()

    def OnUpdate(self):
        pass

    def GenerateColor(self):
        if not self.Enabled: return
        r = self.random.random()
        g = self.random.random()
        b = self.random.random()
        self.material.Set("u_AlbedoColor", Vector3(r, g, b))
