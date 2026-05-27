from Prism import (
    Behaviour, Input, Time, KeyCodes,
    RigidBodyComponent, MeshComponent, TransformComponent,
)
from Prism.Math import Vector3
from Prism.Math.Matrix4 import Matrix4
from Prism.Renderer.Material import MaterialInstance
from RandomColor import RandomColor


class PlayerSphere(Behaviour):
    HorizontalForce: float = 10.0
    JumpForce: float = 10.0
    MaxSpeed: Vector3 = Vector3(0, 0, 0)
    IsEnabled: bool = True

    def OnCreate(self):
        self.m_PhysicsBody = self.GetComponent(RigidBodyComponent)
        self.m_Transform = self.GetComponent(TransformComponent)

        meshComponent = self.GetComponent(MeshComponent)
        self.m_MeshMaterial = meshComponent.Mesh.GetMaterial(0)
        self.m_MeshMaterial.Set("u_Metalness", 0.0)
        self.randomColor = self.CreateComponent(RandomColor)

        self.m_CollisionCounter = 0
        self.IsEnabled = self.Enabled

    @property
    def _colliding(self):
        return self.m_CollisionCounter > 0

    def OnCollisionBegin(self, data):
        self.m_CollisionCounter += 1
        if self._colliding:
            self.m_MeshMaterial.Set("u_AlbedoColor", Vector3(1.0, 0.0, 0.0))

    def OnCollisionEnd(self, data):
        self.m_CollisionCounter -= 1
        if not self._colliding:
            self.randomColor.GenerateColor()

    def OnFixedUpdate(self):
        movementForce = self.HorizontalForce

        if not self._colliding:
            movementForce *= 0.4

        forward = self.m_Transform.Forward
        right = self.m_Transform.Right
        up = self.m_Transform.Up

        if Input.IsKeyPressed(KeyCodes.W):
            self.m_PhysicsBody.AddForce(forward * movementForce)
        elif Input.IsKeyPressed(KeyCodes.S):
            self.m_PhysicsBody.AddForce(forward * -movementForce)

        if Input.IsKeyPressed(KeyCodes.D):
            self.m_PhysicsBody.AddForce(right * movementForce)
        elif Input.IsKeyPressed(KeyCodes.A):
            self.m_PhysicsBody.AddForce(right * -movementForce)

        if self._colliding and Input.IsKeyPressed(KeyCodes.Space):
            self.m_PhysicsBody.AddForce(up * self.JumpForce)

        linearVelocity = self.m_PhysicsBody.GetLinearVelocity()
        linearVelocity.Clamp(Vector3(-self.MaxSpeed.x, -1000, -self.MaxSpeed.z), self.MaxSpeed)
        self.m_PhysicsBody.SetLinearVelocity(linearVelocity)

        if Input.IsKeyPressed(KeyCodes.R):
            transform = self.Entity.GetTransform()
            transform.Translation = Vector3(0.0, 0.0, 0.0)
            self.Entity.SetTransform(transform)
