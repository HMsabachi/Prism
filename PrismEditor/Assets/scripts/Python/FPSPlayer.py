from Prism import (
    Behaviour, Entity, Input, KeyCodes, CursorMode,
    RigidBodyComponent, TransformComponent,
)
from Prism.Math import Vector2, Vector3
from Prism.Math.Mathf import Mathf
from Prism.Core.Time import Time


class FPSPlayer(Behaviour):
    """Python FPS 玩家控制器 — 逻辑与 C# FPSPlayer 一致."""

    WalkingSpeed: float = 10.0
    RunSpeed: float = 20.0
    JumpForce: float = 50.0
    MouseSensitivity: float = 10.0

    _collisionCounter: int = 0

    @property
    def Colliding(self) -> bool:
        return self._collisionCounter > 0

    def OnCreate(self):
        self._transform = self.GetComponent(TransformComponent)
        self._rigidBody = self.GetComponent(RigidBodyComponent)

        self._currentSpeed = self.WalkingSpeed

        self._cameraEntity = Entity.FindEntityByTag("Camera")
        self._cameraTransform = self._cameraEntity.GetComponent(TransformComponent)

        self._cameraRotationX = 0.0

        self._lastMousePosition = Input.GetMousePosition()

        Input.SetCursorMode(CursorMode.Locked)

    def OnUpdate(self):
        ts = Time.DeltaTime

        if Input.IsKeyPressed(KeyCodes.Escape) and Input.GetCursorMode() == CursorMode.Locked:
            Input.SetCursorMode(CursorMode.Normal)

        self._currentSpeed = (
            self.RunSpeed if Input.IsKeyPressed(KeyCodes.LeftControl)
            else self.WalkingSpeed
        )

        self.UpdateRotation(ts)
        self.UpdateMovement()
        self.UpdateCameraTransform()

    def UpdateRotation(self, ts: float):
        currentMousePosition = Input.GetMousePosition()
        delta = self._lastMousePosition - currentMousePosition
        self._rigidBody.Rotate(Vector3(0.0, delta.x * self.MouseSensitivity, 0.0) * ts)

        if delta.y != 0.0:
            self._cameraRotationX += delta.y * self.MouseSensitivity * ts
            #self._cameraRotationX = Mathf.Clamp(self._cameraRotationX, -80.0, 80.0)
            self._cameraTransform.Rotation = Vector3(self._cameraRotationX, 0.0, 0.0)

        self._lastMousePosition = currentMousePosition

    def UpdateMovement(self):
        if Input.IsKeyPressed(KeyCodes.W):
            self._rigidBody.AddForce(self._transform.Forward * self._currentSpeed)
        elif Input.IsKeyPressed(KeyCodes.S):
            self._rigidBody.AddForce(self._transform.Forward * -self._currentSpeed)

        if Input.IsKeyPressed(KeyCodes.A):
            self._rigidBody.AddForce(self._transform.Right * -self._currentSpeed)
        elif Input.IsKeyPressed(KeyCodes.D):
            self._rigidBody.AddForce(self._transform.Right * self._currentSpeed)

        if Input.IsKeyPressed(KeyCodes.Space) and self.Colliding:
            self._rigidBody.AddForce(Vector3.Up * self.JumpForce)

    def UpdateCameraTransform(self):
        # Matrix4 — get a copy, modify Translation, write back
        cameraTransform = self._cameraTransform.Transform
        translation = self._transform.Transform.Translation
        cameraTransform.Translation = Vector3(translation.x, translation.y + 1.5, translation.z)
        self._cameraTransform.Transform = cameraTransform

        # Rotation — compose player yaw with camera pitch/roll
        playerRotation = self._transform.Rotation
        camRotation = self._cameraTransform.Rotation
        self._cameraTransform.Rotation = Vector3(camRotation.x, playerRotation.y, camRotation.z)

    def OnCollisionBegin(self, collision_id: float):
        self._collisionCounter += 1

    def OnCollisionEnd(self, collision_id: float):
        self._collisionCounter -= 1
