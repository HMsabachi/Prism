from Prism.Prism import Behaviour
from Prism.Core import *
from Prism.Component import *
from Prism.Math import *
from Prism.Physics import *
from Prism.Renderer import *
from RandomColor import RandomColor

class PlayerSphere(Behaviour):
    HorizontalForce: float = 10.0
    JumpForce: float = 10.0
    MaxSpeed: Vector3 = Vector3(0, 0, 0)
    IsEnabled: bool = True

    def OnCreate(self):
        self.m_PhysicsBody: RigidBodyComponent = self.GetComponent(RigidBodyComponent)
        self.m_Transform: TransformComponent = self.GetComponent(TransformComponent)
        meshComponent: MeshRendererComponent = self.GetComponent(MeshRendererComponent)
        self.randomColor: RandomColor = self.CreateComponent(RandomColor)
        self.m_MeshMaterial: Material = meshComponent.GetMaterial(0)
        self.m_MeshMaterial.SetFloat("u_Metalness", 0.0)

        self.m_CollisionCounter = 0
        self.IsEnabled = self.Enabled
    
    def OnUpdate(self):
        self.Position = self.m_Transform.Position

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
            self.m_Transform.Position = Vector3(0.0, 0.0, 0.0)
    
    @property
    def _colliding(self):
        return self.m_CollisionCounter > 0

    def OnCollisionBegin(self, data):
        self.m_CollisionCounter += 1
        if self._colliding:
            self.m_MeshMaterial.SetVector3("u_AlbedoColor", Vector3(1.0, 0.0, 0.0))

    def OnCollisionEnd(self, data):
        self.m_CollisionCounter -= 1
        if not self._colliding:
            self.randomColor.GenerateColor()

    def OnTriggerBegin(self, value: float):
        Log.Info(f"[PlayerSphere3D] OnTriggerBegin: {value}")

    def OnTriggerEnd(self, value: float):
        Log.Info(f"[PlayerSphere3D] OnTriggerEnd: {value}")
