from Prism import Behaviour, Time
from Prism.Math import Vector3
from Prism.Math.Matrix4 import Matrix4


class Sink(Behaviour):
    SinkSpeed: float = 0.0

    def OnCreate(self):
        pass

    def OnUpdate(self):
        ts = Time.DeltaTime
        transform = self.Entity.GetTransform()
        translation = transform.Translation

        translation.y -= self.SinkSpeed * ts

        transform.Translation = translation
        self.Entity.SetTransform(transform)
