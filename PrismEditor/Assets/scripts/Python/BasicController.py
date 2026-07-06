from Prism import Behaviour, Entity, Time
from Prism.Math import Vector3
from Prism.Component import TransformComponent


class BasicController(Behaviour):
    Speed: float = 0.0
    DistanceFromPlayer: float = 20.0

    def OnCreate(self):
        self.m_PlayerEntity = Entity.FindEntityByTag("Player")

    def OnUpdate(self):
        ts = Time.DeltaTime

        playerPos = self.m_PlayerEntity.Transform.Position
        myPos = self.Entity.Transform.Position
        myPos.x = playerPos.x
        myPos.y = playerPos.y
        myPos.z = playerPos.z + self.DistanceFromPlayer
        myPos.y = max(myPos.y, 2.0)
        self.Entity.Transform.Position = myPos
