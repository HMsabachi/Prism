from Prism import (
    Behaviour, Entity, Input, KeyCodes,
    RigidBodyComponent, Log
)
from Prism.Math import Vector3


class FPSPlayer(Behaviour):
    """Python FPS 玩家控制器 — 逻辑与 C# FPSPlayer 一致."""

    WalkingSpeed: float = 10.0
    RunSpeed: float = 20.0
    JumpForce: float = 50.0

    _collisionCounter: int = 0
    @property
    def Colliding(self) -> bool:
        return self._collisionCounter > 0

    def __init__(self):
        super().__init__()
        self._current_speed = self.WalkingSpeed
        self._camera_entity = None

    def OnCreate(self):
        self._current_speed = self.WalkingSpeed
        self._camera_entity = Entity.FindEntityByTag("Camera")

    def OnFixedUpdate(self):
        self._current_speed = (
            self.RunSpeed if Input.IsKeyPressed(KeyCodes.LeftControl)
            else self.WalkingSpeed
        )

        forward = self.Entity.Transform.Forward
        right = self.Entity.Transform.Right
        up = self.Entity.Transform.Up

        rb = self.GetComponent(RigidBodyComponent);

        if Input.IsKeyPressed(KeyCodes.W):
            rb.AddForce(forward * self._current_speed)
        elif Input.IsKeyPressed(KeyCodes.S):
            rb.AddForce(forward * -self._current_speed)

        if Input.IsKeyPressed(KeyCodes.A):
            rb.AddForce(right * -self._current_speed)
        elif Input.IsKeyPressed(KeyCodes.D):
            rb.AddForce(right * self._current_speed)

        if Input.IsKeyPressed(KeyCodes.Space) and self.Colliding:
            rb.AddForce(up * self.JumpForce)

    def OnUpdate(self):
        if self._camera_entity is None:
            return

        # TODO: 这个工作流程需要改进（将通过对象父子关系解决）
        camera_transform = self._camera_entity.GetTransform()
        camera_translation = camera_transform.Translation
        player_translation = self.Entity.GetTransform().Translation

        camera_translation.x = player_translation.x
        camera_translation.z = player_translation.z
        camera_translation.y = player_translation.y + 1.5

        camera_transform.Translation = camera_translation
        self._camera_entity.SetTransform(camera_transform)

    def OnCollisionBegin(self, collision_id: float):
        self._collisionCounter += 1

    def OnCollisionEnd(self, collision_id: float):
        self._collisionCounter -= 1

