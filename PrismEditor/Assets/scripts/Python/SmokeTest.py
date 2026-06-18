import PrismNative as _Prism
from Prism import (
    Behaviour, Log, Input, Time, KeyCodes,
    TagComponent, TransformComponent, MeshRendererComponent, RigidBodyComponent,
)
from Prism.Math import Vector2, Vector3, Vector4, Quaternion, Mathf


class SmokeTest(Behaviour):

    # 公开字段
    TestFloat: float = 3.14
    TestInt: int = 42
    TestVec2: Vector2 = Vector2(1.0, 2.0)
    TestVec3: Vector3 = Vector3(1.0, 2.0, 3.0)
    TestVec4: Vector4 = Vector4(1.0, 2.0, 3.0, 4.0)
    TestQuat: Quaternion = Quaternion(0.0, 0.0, 0.0, 1.0)

    def __init__(self):
        super().__init__()
        self._UpdateCount = 0
        self._DestroyCalled = False

    def OnCreate(self):
        Log.Info("[SmokeTest] OnCreate 开始")

        self._TestLog()

        self._TestTime()

        self._TestInput()

        self._TestEntity()

        self._TestTransform()

        self._TestPublicFields()

        Log.Info("[SmokeTest] === 冒烟测试通过 ===")

    def OnUpdate(self):
        self._UpdateCount += 1
        dt = Time.DeltaTime
        elapsed = Time.Time


        if self._UpdateCount % 60 == 0:
            Log.Info(f"[SmokeTest] 运行中: UpdateCount={self._UpdateCount}, "
                     f"DeltaTime={dt:.4f}, Time={elapsed:.2f}")

    def OnDestroy(self):
        self._DestroyCalled = True
        Log.Info(f"[SmokeTest] OnDestroy, UpdateCount={self._UpdateCount}")

    # ─── 测试方法 ───────────────────────────────

    def _TestLog(self):
        Log.Trace("[SmokeTest] Log.Trace")
        Log.Debug("[SmokeTest] Log.Debug")
        Log.Info("[SmokeTest] Log.Info")
        Log.Warn("[SmokeTest] Log.Warn")
        Log.Error("[SmokeTest] Log.Error")
        Log.Critical("[SmokeTest] Log.Critical")
        Log.Info("[SmokeTest] ✓ Log API")

    def _TestTime(self):
        dt = Time.DeltaTime
        t = Time.Time
        assert dt >= 0.0, f"DeltaTime 不应为负: {dt}"
        assert t >= 0.0, f"Time 不应为负: {t}"
        Log.Info(f"[SmokeTest] ✓ Time API: dt={dt:.6f}, time={t:.2f}")

    def _TestInput(self):
        space = Input.IsKeyPressed(KeyCodes.Space)
        Log.Info(f"[SmokeTest] ✓ Input API: Space={space}")

    def _TestEntity(self):
        if self.Entity is None:
            Log.Warn("[SmokeTest] 跳过 Entity API: Entity 未设置")
            return

        eid = self.Entity.ID

        hasTag = _Prism.Prism_Entity_HasComponent(eid, TagComponent)
        hasMesh = _Prism.Prism_Entity_HasComponent(eid, MeshRendererComponent)
        hasRigid = _Prism.Prism_Entity_HasComponent(eid, RigidBodyComponent)

        Log.Info(f"[SmokeTest] ✓ Entity API: HasTag={hasTag}, HasMesh={hasMesh}, "
                 f"HasRigidBody={hasRigid}")

    def _TestTransform(self):
        if self.Entity is None:
            Log.Warn("[SmokeTest] 跳过 Transform API: Entity 未设置")
            return

        eid = self.Entity.ID

        _Prism.Prism_TransformComponent_SetPosition(eid, (0.0, 0.0, 0.0))

        pos = _Prism.Prism_TransformComponent_GetPosition(eid)
        assert len(pos) == 3, f"位置 tuple 长度应为 3, 实际 {len(pos)}"
        x, y, z = pos
        Log.Info(f"[SmokeTest] ✓ Transform: position=({x:.1f}, {y:.1f}, {z:.1f})")
        assert abs(x - 0.0) < 0.001, f"x 应为 0.0, 实际 {x}"
        assert abs(y - 0.0) < 0.001, f"y 应为 0.0, 实际 {y}"
        assert abs(z - 0.0) < 0.001, f"z 应为 0.0, 实际 {z}"

        mat = _Prism.Prism_Entity_GetTransform(eid)
        assert len(mat) == 16, f"矩阵 tuple 长度应为 16, 实际 {len(mat)}"
        Log.Info(f"[SmokeTest] ✓ Transform: GetTransform 返回 16 元素矩阵")

    def _TestPublicFields(self):
        Log.Info(f"[SmokeTest] Public fields: "
                 f"TestFloat={self.TestFloat}, "
                 f"TestInt={self.TestInt}")
        assert self.TestFloat != 0, "TestFloat 不应为 0"
        assert self.TestInt != 0, "TestInt 不应为 0"
        Log.Info(f"[SmokeTest] ✓ Public fields")
