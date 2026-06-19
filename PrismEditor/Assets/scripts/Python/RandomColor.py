import random
from Prism import Behaviour, MeshRendererComponent, Time, Log
from Prism.Math import Vector3


class RandomColor(Behaviour):
    def OnCreate(self):
        self.random = random.Random()
        meshComponent = self.GetComponent(MeshRendererComponent)
        self.material = meshComponent.GetMaterial(0)
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
        self.material.SetVector3("u_AlbedoColor", Vector3(r, g, b))
