import math
from Prism import Behaviour, Input, Time, KeyCodes, Log
from Prism.Math import Vector3
from Prism.Math.Matrix4 import Matrix4


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
        transform = self.Entity.GetTransform()
        translation = transform.Translation
        translation.x = math.sin(self.time) * self.Speed
        transform.Translation = translation
        self.Entity.SetTransform(transform)
