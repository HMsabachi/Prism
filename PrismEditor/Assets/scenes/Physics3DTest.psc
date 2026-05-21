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
  - Entity: 3509336336274569647
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [0.57714844, 3.7860096, -9.399623]
      Rotation: [1, 0, 0, 0]
      Scale: [0.022940006, 0.022940006, 0.022940006]
    MeshComponent:
      AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\models\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      IsKinematic: false
      LockPositionX: false
      LockPositionY: false
      LockPositionZ: false
      LockRotationX: false
      LockRotationY: false
      LockRotationZ: false
    MeshColliderComponent:
      AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\models\Sphere1m.fbx
  - Entity: 10169503531257462571
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [0, 1.5, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    ScriptsComponent:
      - ModuleName: Example.PlayerSphere
        Language: 0
        StoredFields:
          - Name: HorizontalForce
            Type: 1
            Data: 10
          - Name: MaxSpeed
            Type: 6
            Data: [0, 0, 0]
          - Name: JumpForce
            Type: 1
            Data: 10
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.5
      IsKinematic: false
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
      Size: [2, 2, 2]
      Offset: [0, 0, 0]
  - Entity: 14057422478420564497
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [-19.43363, 4.5087404, -1.9669533e-06]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    ScriptsComponent:
      - ModuleName: Example.PlayerSphere
        Language: 0
        StoredFields:
          - Name: HorizontalForce
            Type: 1
            Data: 100
          - Name: MaxSpeed
            Type: 6
            Data: [10, 10, 10]
          - Name: JumpForce
            Type: 1
            Data: 200
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      IsKinematic: false
      LockPositionX: false
      LockPositionY: false
      LockPositionZ: false
      LockRotationX: true
      LockRotationY: true
      LockRotationZ: true
    PhysicsMaterialComponent:
      StaticFriction: 1
      DynamicFriction: 1
      Bounciness: 0
    SphereColliderComponent:
      Radius: 0.5
  - Entity: 5178862374589434728
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [-21.740631, 9.706595, 15]
      Rotation: [0.99991035, -0.013391121, 0, 0]
      Scale: [1, 1, 1]
    ScriptsComponent:
      - ModuleName: Example.BasicController
        Language: 0
        StoredFields:
          - Name: Speed
            Type: 1
            Data: 12
          - Name: DistanceFromPlayer
            Type: 1
            Data: 15
    CameraComponent:
      Camera: some camera data...
      Primary: true
  - Entity: 18306113171518048249
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [50, 1, 50]
    ScriptsComponent:
      - ModuleName: SmokeTest
        Language: 1
        StoredFields:
          - Name: TestFloat
            Type: 1
            Data: 3.14
          - Name: TestInt
            Type: 2
            Data: 42
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 0
      Mass: 1
      IsKinematic: false
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
      Size: [50, 1, 50]
      Offset: [0, 0, 0]