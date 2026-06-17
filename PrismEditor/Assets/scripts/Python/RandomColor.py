import random
from Prism import Behaviour, MeshComponent, Time, Log
from Prism.Math import Vector3


class RandomColor(Behaviour):
    def OnCreate(self):
        self.random = random.Random()
        meshComponent = self.GetComponent(MeshComponent)
        self.material = meshComponent.Mesh.GetMaterial(0)
        self.GenerateColor()
        self._timer = 0.0

    def OnUpdate(self):
        self._timer += Time.DeltaTime
        if self._timer >= 1.0:
            self._timer = 0.0
            self.GenerateColor()
        pass

    def GenerateColor(self):
        if not self.Enabled: return
        r = self.random.random()
        g = self.random.random()
        b = self.random.random()
        self.material.Set("u_AlbedoColor", Vector3(r, g, b))
