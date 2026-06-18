from Prism import (
    Behaviour, Input, Time, KeyCodes,
    RigidBody2DComponent, MeshRendererComponent,
)
from Prism.Math import Vector2, Vector3
from Prism.Renderer.Material import Material


class PlayerCube(Behaviour):
    HorizontalForce: float = 10.0
    JumpForce: float = 10.0
    Velocity: Vector2 = Vector2(0, 0)
    MaxSpeed: Vector2 = Vector2(0, 0)

    def OnCreate(self):
        self.m_PhysicsBody = self.GetComponent(RigidBody2DComponent)

        meshComponent = self.GetComponent(MeshRendererComponent)
        self.m_MeshMaterial = meshComponent.GetMaterial(0)
        self.m_MeshMaterial.Set("u_Metalness", 0.0)

        self.m_CollisionCounter = 0

    def OnCollisionBegin(self, data):
        self.m_CollisionCounter += 1

    def OnCollisionEnd(self, data):
        self.m_CollisionCounter -= 1

    @property
    def _colliding(self):
        return self.m_CollisionCounter > 0

    def OnFixedUpdate(self):
        ts = Time.FixedDeltaTime
        movementForce = self.HorizontalForce
        self.Velocity = self.m_PhysicsBody.LinearVelocity
        if not self._colliding:
            movementForce *= 0.4

        if Input.IsKeyPressed(KeyCodes.D):
            self.m_PhysicsBody.ApplyLinearImpulse(Vector2(movementForce, 0), Vector2(), True)
        elif Input.IsKeyPressed(KeyCodes.A):
            self.m_PhysicsBody.ApplyLinearImpulse(Vector2(-movementForce, 0), Vector2(), True)

        if self._colliding and Input.IsKeyPressed(KeyCodes.Space):
            self.m_PhysicsBody.ApplyLinearImpulse(Vector2(0, self.JumpForce), Vector2(0, 0), True)

        if self.m_CollisionCounter > 0:
            self.m_MeshMaterial.Set("u_AlbedoColor", Vector3(1.0, 0.0, 0.0))
        else:
            self.m_MeshMaterial.Set("u_AlbedoColor", Vector3(0.8, 0.8, 0.8))
