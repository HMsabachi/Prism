from Prism.Entity import Entity
from Prism.Behaviour import Behaviour
from Prism.Component import TransformComponent, RigidBodyComponent, MeshRendererComponent
from Prism.Physics.Collider import BoxCollider, SphereCollider, CapsuleCollider, MeshCollider
from Prism.Physics.Physics import Physics, RaycastHit
from Prism.Core.Input import Input, CursorMode, MouseButton
from Prism.Core.KeyCodes import KeyCodes
from Prism.Core.Log import Log
from Prism.Math import Vector2, Vector3
from Prism.Math.Mathf import Mathf
from Prism.Core.Time import Time


class FPSPlayer(Behaviour):
    """Python FPS 玩家控制器"""

    WalkingSpeed: float = 10.0
    RunSpeed: float = 20.0
    JumpForce: float = 50.0
    MouseSensitivity: float = 10.0

    _collisionCounter: int = 0

    @property
    def Colliding(self) -> bool:
        return self._collisionCounter > 0

    def OnCreate(self):
        self._transform: TransformComponent = self.GetComponent(TransformComponent)
        self._rigidBody: RigidBodyComponent = self.GetComponent(RigidBodyComponent)
        self._cameraEntity = Entity.FindEntityByTag("Camera")
        self._cameraTransform: TransformComponent = self._cameraEntity.GetComponent(TransformComponent)

        self._currentSpeed = self.WalkingSpeed

        self._lastMousePosition = Input.GetMousePosition()

        Input.SetCursorMode(CursorMode.Locked)

    def OnFixedUpdate(self):
        ts = Time.DeltaTime

        if Input.IsKeyPressed(KeyCodes.Escape) and Input.GetCursorMode() == CursorMode.Locked:
            Input.SetCursorMode(CursorMode.Normal)

        # if Input.IsMouseButtonPressed(MouseButton.Left) and Input.GetCursorMode() == CursorMode.Normal:
        #     Input.SetCursorMode(CursorMode.Locked)

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
        yRotation = delta.x * self.MouseSensitivity * ts
        xRotation = delta.y * self.MouseSensitivity * ts
        self._rigidBody.Rotate(Vector3(0.0, yRotation, 0.0))

        if delta.y != 0.0 or delta.x != 0.0:
            self._cameraTransform.Rotation += Vector3(xRotation, yRotation, 0.0)

        self._cameraTransform.Rotation = Vector3(Mathf.Clamp(self._cameraTransform.Rotation.x, -80.0, 80.0), self._cameraTransform.Rotation.YZ)

        self._lastMousePosition = currentMousePosition

    def UpdateMovement(self):
        if Input.IsKeyPressed(KeyCodes.H):
            origin = self._cameraTransform.Position + (self._cameraTransform.Transform.Forward * 5.0)
            hit = RaycastHit()
            if Physics.Raycast(origin, self._cameraTransform.Transform.Forward, 20.0, hit):
                entity = Entity.FindEntityByID(hit.EntityID)
                if entity is not None:
                    mesh: MeshRendererComponent = entity.GetComponent(MeshRendererComponent)
                    if mesh is not None and mesh.Mesh is not None:
                        mesh.GetMaterial(0).SetFloat("u_Metalness", 1.0)
        if Input.IsKeyPressed(KeyCodes.L):
            colliders = Physics.OverlapBox(self._transform.Position, Vector3(1.0, 1.0, 1.0))
            Log.Trace("Overlap count: {}", len(colliders))
            for c in colliders:
                Log.Trace("EntityID: {}", c.EntityID)
                Log.Trace("IsTrigger: {}", c.IsTrigger)
                Log.Trace("IsBox: {}", isinstance(c, BoxCollider))
                Log.Trace("IsSphere: {}", isinstance(c, SphereCollider))
                Log.Trace("IsCapsule: {}", isinstance(c, CapsuleCollider))
                Log.Trace("IsMesh: {}", isinstance(c, MeshCollider))

        if Input.IsKeyPressed(KeyCodes.W):
            self._rigidBody.AddForce(self._cameraTransform.Transform.Forward * self._currentSpeed)
        elif Input.IsKeyPressed(KeyCodes.S):
            self._rigidBody.AddForce(self._cameraTransform.Transform.Forward * -self._currentSpeed)

        if Input.IsKeyPressed(KeyCodes.A):
            self._rigidBody.AddForce(self._cameraTransform.Transform.Right * -self._currentSpeed)
        elif Input.IsKeyPressed(KeyCodes.D):
            self._rigidBody.AddForce(self._cameraTransform.Transform.Right * self._currentSpeed)

        if Input.IsKeyPressed(KeyCodes.Space) and self.Colliding:
            self._rigidBody.AddForce(Vector3.Up * self.JumpForce)
        velocity = self._rigidBody.GetLinearVelocity()
        velocity.Clamp(Vector3(-self._currentSpeed), Vector3(self._currentSpeed))

    def UpdateCameraTransform(self):
        position = Vector3(self._transform.Position)
        position.y += 1.5
        self._cameraTransform.Position = position

    def OnCollisionBegin(self, collision_id: float):
        self._collisionCounter += 1

    def OnCollisionEnd(self, collision_id: float):
        self._collisionCounter -= 1
