import math
from Prism import Behaviour, Input, Time, KeyCodes, Log
from Prism.Math import Vector3


class Script(Behaviour):
    Speed: float = 5.0

    def OnCreate(self):
        self.time = 0.0
        Log.Trace("Script.OnCreate")

    def OnUpdate(self):
        if not Input.IsKeyPressed(KeyCodes.Space):
            return
        self.time += Time.DeltaTime
        if self.time > 100.0:
            self.time = 0.0

        pos = self.Entity.Transform.Position
        pos.x = math.sin(self.time) * self.Speed
        self.Entity.Transform.Position = pos
