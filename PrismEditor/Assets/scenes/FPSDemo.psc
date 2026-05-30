Scene: Scene Name
Environment:
  AssetPath: assets\env\pink_sunrise_4k.hdr
  Light:
    Direction: [-0.608, -1, 0.552]
    Radiance: [1, 1, 1]
    Multiplier: 0.515
  Shadow:
    Enabled: true
    Bias: 0.001
    NormalBias: 0.1
    CascadeCount: 4
Entities:
  - Entity: 18306113171518048249
    TagComponent:
      Tag: Ground
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [50, 1, 50]
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 0
      Mass: 1
      IsKinematic: false
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: false
        LockRotationY: false
        LockRotationZ: false
    PhysicsMaterialComponent:
      StaticFriction: 1
      DynamicFriction: 1
      Bounciness: 0
    BoxColliderComponent:
      Size: [1, 1, 1]
      Offset: [0, 0, 0]
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 5178862374589434728
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [2.808, 2.25, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    CameraComponent:
      Camera: some camera data...
      Primary: true
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 3247025703490125974
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [2.8080375, 1.5, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    MeshComponent:
      AssetPath: Assets\meshes\Capsule.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      IsKinematic: false
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: true
        LockRotationY: false
        LockRotationZ: true
    PhysicsMaterialComponent:
      StaticFriction: 0.1
      DynamicFriction: 0.1
      Bounciness: 0.1
    MeshColliderComponent:
      AssetPath: Assets\meshes\Capsule.fbx
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        - ID: 4359666882938965819
          ClassID: 4621281492369714213
          Enabled: true
          Fields:
            - ID: 316217235
              Name: WalkingSpeed
              Type: 1
              Value: 10
            - ID: 2138685965
              Name: RunSpeed
              Type: 1
              Value: 20
            - ID: 868800460
              Name: JumpForce
              Type: 1
              Value: 50
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        - ID: 2411706344143003445
          ClassID: 14374398382941118209
          Enabled: false
          Fields:
            - ID: 316217235
              Name: WalkingSpeed
              Type: 1
              Value: 10
            - ID: 2138685965
              Name: RunSpeed
              Type: 1
              Value: 20
            - ID: 868800460
              Name: JumpForce
              Type: 1
              Value: 50
  - Entity: 11149966982516343187
    TagComponent:
      Tag: Mesh Collider
    TransformComponent:
      Position: [-2.6046, 1, -0.0016999245]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.1
      IsKinematic: false
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: false
        LockRotationY: false
        LockRotationZ: false
    PhysicsMaterialComponent:
      StaticFriction: 1
      DynamicFriction: 1
      Bounciness: 0.1
    MeshColliderComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 10169503531257462571
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [0, 1.5, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.5
      IsKinematic: false
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: false
        LockRotationY: false
        LockRotationZ: false
    PhysicsMaterialComponent:
      StaticFriction: 1
      DynamicFriction: 1
      Bounciness: 0
    BoxColliderComponent:
      Size: [1, 1, 1]
      Offset: [0, 0, 0]
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 14057422478420564497
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [-3.9877, 1, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      IsKinematic: false
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: true
        LockRotationY: true
        LockRotationZ: true
    PhysicsMaterialComponent:
      StaticFriction: 0.1
      DynamicFriction: 0.1
      Bounciness: 0.1
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []