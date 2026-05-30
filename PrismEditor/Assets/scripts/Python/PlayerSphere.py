
from Prism import (
    Behaviour, Entity, Log, Input, Time, KeyCodes, Mathf,
    RigidBodyComponent, RigidBody2DComponent, TagComponent,
    TransformComponent,
    ForceMode,
)
from Prism.Math import Vector2, Vector3, Vector4, Quaternion


class PlayerSphere(Behaviour):
    """Python 版 PlayerSphere — 测试常用 API 是否正常."""

    # 公开字段
    HorizontalForce: float = 10.0
    JumpForce: float = 10.0
    MaxSpeed: Vector3 = Vector3(10.0, 1000.0, 10.0)

    TestFloat: float = 3.14
    TestInt: int = 42
    TestVec2: Vector2 = Vector2(1.0, 2.0)
    TestVec3: Vector3 = Vector3(1.0, 2.0, 3.0)
    TestVec4: Vector4 = Vector4(1.0, 2.0, 3.0, 4.0)
    TestQuat: Quaternion = Quaternion(0.0, 0.0, 0.0, 1.0)

    def __init__(self):
        super().__init__()
        self._update_count = 0
        self._collision_counter = 0

    # ─── 生命周期 ───────────────────────────────

    def OnCreate(self):
        Log.Info("[PlayerSphere] OnCreate — 测试 API 开始")

        self._TestCore()
        self._TestTime()
        self._TestMath()
        self._TestTransform()
        self._TestEntity()
        self._TestVector()

        Log.Info("[PlayerSphere] === 基础 API 测试通过 ===")

    def OnUpdate(self):
        self._update_count += 1
        dt = Time.DeltaTime
        elapsed = Time.Time
        if self._update_count % 120 == 0:
            Log.Trace(f"[PlayerSphere] 运行中: frame={Time.FrameCount}, "
                      f"dt={dt:.4f}, time={elapsed:.2f}")

    def OnFixedUpdate(self):
        """测试 RigidBody + Input + Transform """
        if self.Entity is None:
            return

        movement = self.HorizontalForce

        if self._collision_counter <= 0:
            movement *= 0.4

        forward = self.Entity.Transform.Forward
        right = self.Entity.Transform.Right
        up = self.Entity.Transform.Up

        rb = self._GetRigidBody()
        if rb is None:
            return

        if Input.IsKeyPressed(KeyCodes.W):
            rb.AddForce(forward * movement)
        elif Input.IsKeyPressed(KeyCodes.S):
            rb.AddForce(forward * -movement)

        if Input.IsKeyPressed(KeyCodes.D):
            rb.AddForce(right * movement)
        elif Input.IsKeyPressed(KeyCodes.A):
            rb.AddForce(right * -movement)

        if self._collision_counter > 0 and Input.IsKeyPressed(KeyCodes.Space):
            rb.AddForce(up * self.JumpForce)

        vel = rb.GetLinearVelocity()
        vel.Clamp(vel, -self.MaxSpeed, self.MaxSpeed)
        rb.SetLinearVelocity(vel)

        if Input.IsKeyPressed(KeyCodes.R):
            transform = _GetTransformMatrix(self.Entity._id)
            _SetTransformMatrix(self.Entity._id, transform)


    def OnCollisionBegin(self, collision_id: float):
        self._collision_counter += 1
        Log.Info(f"[PlayerSphere] OnCollisionBegin: {collision_id}")

    def OnCollisionEnd(self, collision_id: float):
        self._collision_counter -= 1
        Log.Trace(f"[PlayerSphere] OnCollisionEnd: {collision_id}")

    def OnTriggerBegin(self, value: float):
        Log.Info(f"[PlayerSphere] OnTriggerBegin: {value}")

    def OnTriggerEnd(self, value: float):
        Log.Info(f"[PlayerSphere] OnTriggerEnd: {value}")


    def _TestCore(self):
        """测试 Log / Input / Time / KeyCodes"""
        Log.Trace("[PlayerSphere] Log.Trace")
        Log.Info("[PlayerSphere] Log.Info")
        Log.Warn("[PlayerSphere] Log.Warn")
        # Log.Error / Log.Critical 故意不测，避免污染输出

        space = Input.IsKeyPressed(KeyCodes.Space)
        Log.Info(f"[PlayerSphere] Input.IsKeyPressed(Space) = {space}")

    def _TestTime(self):
        """测试 Time 类级属性"""
        dt = Time.DeltaTime
        t = Time.Time
        udt = Time.UnscaledDeltaTime
        ut = Time.UnscaledTime
        fdt = Time.FixedDeltaTime
        fc = Time.FrameCount
        ts = Time.TimeScale

        assert dt >= 0.0, f"DeltaTime = {dt}"
        assert t >= 0.0, f"Time = {t}"
        assert fc >= 0, f"FrameCount = {fc}"
        Log.Info(f"[PlayerSphere] ✓ Time: dt={dt:.6f} scale={ts:.2f} frame={fc}")

    def _TestMath(self):
        """测试 Mathf + Vector + Quaternion"""
        assert Mathf.PI > 3.14
        assert Mathf.Deg2Rad < 0.02
        assert abs(Mathf.Sin(0.0)) < 0.001
        assert abs(Mathf.Cos(0.0) - 1.0) < 0.001
        assert Mathf.Clamp(5, 0, 1) == 1
        assert Mathf.Lerp(0, 10, 0.5) == 5.0
        assert Mathf.Approximately(1.0, 1.0000001)
        Log.Info("[PlayerSphere] ✓ Mathf")

    def _TestTransform(self):
        """测试 Transform 属性访问"""
        if self.Entity is None:
            Log.Warn("[PlayerSphere] 跳过 Transform: Entity 为 None")
            return

        t = self.Entity.Transform
        pos = t.Position
        rot = t.Rotation
        s = t.Scale
        Log.Info(f"[PlayerSphere] ✓ Transform: pos=({pos.x:.2f}, {pos.y:.2f}, {pos.z:.2f})")

        fwd = t.Forward
        r = t.Right
        u = t.Up
        assert abs(fwd.Magnitude - 1.0) < 0.001, f"Forward 非单位向量: {fwd}"
        assert abs(r.Magnitude - 1.0) < 0.001, f"Right 非单位向量: {r}"
        assert abs(u.Magnitude - 1.0) < 0.001, f"Up 非单位向量: {u}"

        # Position get/set
        old = Vector3(pos)
        t.Position = Vector3(1.0, 2.0, 3.0)
        assert abs(t.Position.x - 1.0) < 0.001
        t.Position = old
        Log.Info("[PlayerSphere] ✓ Transform: Position/Forward/Right/Up")

    def _TestRigidBody(self):
        """测试 RigidBodyComponent 创建和读写"""
        if self.Entity is None:
            return

        # 确保有 RigidBodyComponent
        has_rb = self.Entity.HasComponent(RigidBodyComponent)
        if not has_rb:
            Log.Warn("[PlayerSphere] 跳过 RigidBody: 实体没有 RigidBodyComponent")
            return

        rb = self._GetRigidBody()
        if rb is None:
            return

        # Get/Set LinearVelocity
        old_vel = rb.GetLinearVelocity()
        rb.SetLinearVelocity(Vector3(5.0, 0.0, 0.0))
        new_vel = rb.GetLinearVelocity()
        Log.Info(f"[PlayerSphere] ✓ RigidBody: velocity=({new_vel.x:.2f}, {new_vel.y:.2f}, {new_vel.z:.2f})")
        rb.SetLinearVelocity(old_vel)

        # AddForce
        rb.AddForce(Vector3(0.0, 10.0, 0.0))
        Log.Info("[PlayerSphere] ✓ RigidBody: AddForce")

    def _TestEntity(self):
        """测试 Entity API"""
        if self.Entity is None:
            return

        eid = self.Entity.ID
        assert eid != 0, "Entity.ID 不应为 0"
        Log.Info(f"[PlayerSphere] ✓ Entity: ID={eid}")

        # HasComponent
        assert self.Entity.HasComponent(TransformComponent)
        assert self.Entity.HasComponent(TagComponent)
        Log.Info(f"[PlayerSphere] ✓ Entity: HasComponent")

        # TagComponent
        tag = self.GetEntityTag()
        Log.Info(f"[PlayerSphere] ✓ TagComponent: Tag='{tag}'")

    def _TestVector(self):
        """测试 Vector2/3/4 常用 API"""
        # Vector3
        v = Vector3(1.0, 2.0, 3.0)
        assert abs(v.Magnitude - 3.741657) < 0.001
        assert abs(Mathf.Dot(v, Vector3.Right) - 1.0) < 0.001
        cross = Mathf.Cross(v, Vector3.Up)
        assert abs(cross.x + 3.0) < 0.001
        n = v.Normalized
        assert abs(n.Magnitude - 1.0) < 0.001

        # Vector2
        v2 = Vector2(3.0, 4.0)
        assert abs(v2.Magnitude - 5.0) < 0.001

        # Vector4
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        assert abs(v4.Magnitude - 5.477225) < 0.01

        # Quaternion
        q = Quaternion.Euler(0.0, 90.0, 0.0)
        rotated = q * Vector3.Forward
        assert abs(rotated.x - 1.0) < 0.01
        Log.Info("[PlayerSphere] ✓ Vector2/3/4 + Quaternion")

    # ─── 辅助 ───────────────────────────────────

    def _GetRigidBody(self):
        if not hasattr(self, '_rb_cache'):
            try:
                self.Entity.GetComponent(RigidBodyComponent)
            except Exception:
                pass
            self._rb_cache = RigidBodyComponent()
            self._rb_cache.Entity = self.Entity
        return self._rb_cache

    def GetEntityTag(self) -> str:
        tag = TagComponent()
        tag.Entity = self.Entity
        return tag.Tag


def _GetTransformMatrix(entity_id: int):
    """测试 GetTransform (mat4)."""
    import PrismNative as _Prism
    return _Prism.Prism_Entity_GetTransform(entity_id)


def _SetTransformMatrix(entity_id: int, mat):
    """测试 SetTransform (mat4)."""
    import PrismNative as _Prism
    _Prism.Prism_Entity_SetTransform(entity_id, mat)
