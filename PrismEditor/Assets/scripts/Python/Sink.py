from Prism import Behaviour, Time
from Prism.Math import Vector3


class Sink(Behaviour):
    SinkSpeed: float = 0.0

    def OnCreate(self):
        pass

    def OnUpdate(self):
        ts = Time.DeltaTime

        pos = self.Entity.Transform.Position
        pos.y -= self.SinkSpeed * ts
        self.Entity.Transform.Position = pos
