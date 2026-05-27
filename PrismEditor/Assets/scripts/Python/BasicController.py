from Prism import Behaviour, Entity, Time
from Prism.Math import Vector3
from Prism.Math.Matrix4 import Matrix4


class BasicController(Behaviour):
    Speed: float = 0.0
    DistanceFromPlayer: float = 20.0

    def OnCreate(self):
        self.m_PlayerEntity = Entity.FindEntityByTag("Player")

    def OnUpdate(self):
        ts = Time.DeltaTime
        transform = self.Entity.GetTransform()

        playerTranslation = self.m_PlayerEntity.GetTransform().Translation
        translation = transform.Translation
        translation.XY = playerTranslation.XY
        translation.z = playerTranslation.z + self.DistanceFromPlayer
        translation.y = max(translation.y, 2.0)
        transform.Translation = translation
        self.Entity.SetTransform(transform)
