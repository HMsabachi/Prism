Scene: Scene Name
Environment:
  AssetPath: assets\env\pink_sunrise_4k.hdr
  Light:
    Direction: [-0.608, -1, 0.552]
    Radiance: [1, 1, 1]
    Multiplier: 0.515
  Shadow:
    Enabled: true
    Bias: 0
    NormalBias: 0.155
    CascadeCount: 4
    MaxDistance: 180.979
PhysicsLayers:
  - Name: Default
    CollidesWith:
      - Name: Player
      - Name: Ground
  - Name: Player
    CollidesWith:
      - Name: Default
      - Name: Ground
  - Name: Ground
    CollidesWith:
      - Name: Default
      - Name: Player
Entities:
  - Entity: 14057422478420564497
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [-3.9877, 1, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      IsKinematic: false
      Layer: 0
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
        - ClassID: 17473292295039821981
          Enabled: true
          Fields:
            - ID: 2824397154
              Name: Timer
              Type: 1
              Value: 0
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
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
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.5
      IsKinematic: false
      Layer: 0
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
        - ClassID: 17473292295039821981
          Enabled: true
          Fields:
            - ID: 2824397154
              Name: Timer
              Type: 1
              Value: 0
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 11149966982516343187
    TagComponent:
      Tag: Mesh Collider
    TransformComponent:
      Position: [-2.6046, 1, -0.0016999245]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.1
      IsKinematic: false
      Layer: 0
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
        - ClassID: 17473292295039821981
          Enabled: true
          Fields:
            - ID: 2824397154
              Name: Timer
              Type: 1
              Value: 0
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
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
    MeshRendererComponent:
      AssetPath: Assets\meshes\Capsule.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      IsKinematic: false
      Layer: 1
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
        - ClassID: 17473292295039821981
          Enabled: true
          Fields:
            - ID: 2824397154
              Name: Timer
              Type: 1
              Value: 0
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
        - ClassID: 4621281492369714213
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
            - ID: 1438023815
              Name: MouseSensitivity
              Type: 1
              Value: 10
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        - ClassID: 14374398382941118209
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
            - ID: 2665483710
              Name: _collisionCounter
              Type: 6
              Value: 0
            - ID: 868800460
              Name: JumpForce
              Type: 1
              Value: 50
            - ID: 1438023815
              Name: MouseSensitivity
              Type: 1
              Value: 10
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
  - Entity: 18306113171518048249
    TagComponent:
      Tag: Ground
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [50, 1, 50]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 0
      Mass: 1
      IsKinematic: false
      Layer: 2
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
        - ClassID: 17473292295039821981
          Enabled: true
          Fields:
            - ID: 2824397154
              Name: Timer
              Type: 1
              Value: 0
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []