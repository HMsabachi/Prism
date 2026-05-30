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
      Tag: Box
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
      Size: [50, 1, 50]
      Offset: [0, 0, 0]
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        - ID: 12040501226740582726
          ClassID: 17473292295039821981
          Enabled: true
          Fields:
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 5178862374589434728
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [-21.740631, 9.706595, 15]
      Rotation: [0.99991035, -0.013391121, 0, 0]
      Scale: [1, 1, 1]
    CameraComponent:
      Camera: some camera data...
      Primary: true
    CSharpScriptComponent:
      Behaviours:
        - ID: 5974780048155238521
          ClassID: 502499430819245105
          Enabled: true
          Fields:
            - ID: 305488032
              Name: Speed
              Type: 1
              Value: 5.6
            - ID: 3277313775
              Name: DistanceFromPlayer
              Type: 1
              Value: 10.8
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 14057422478420564497
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [-19.43363, 4.5087404, -1.9669533e-06]
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
      StaticFriction: 1
      DynamicFriction: 1
      Bounciness: 0
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        - ID: 7703927552313077461
          ClassID: 14697641082096727597
          Enabled: true
          Fields:
            - ID: 1693159688
              Name: HorizontalForce
              Type: 1
              Value: 80
            - ID: 868800460
              Name: JumpForce
              Type: 1
              Value: 220
            - ID: 414202020
              Name: MaxSpeed
              Type: 13
              Value: [20, 20, 20]
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        - ID: 8416391071943564156
          ClassID: 16452152310796986188
          Enabled: false
          Fields:
            - ID: 1693159688
              Name: HorizontalForce
              Type: 1
              Value: 80
            - ID: 868800460
              Name: JumpForce
              Type: 1
              Value: 220
            - ID: 414202020
              Name: MaxSpeed
              Type: 13
              Value: [20, 20, 20]
            - ID: 882838894
              Name: IsEnabled
              Type: 3
              Value: true
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
      Size: [2, 2, 2]
      Offset: [0, 0, 0]
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        - ID: 7801379288073613711
          ClassID: 17473292295039821981
          Enabled: true
          Fields:
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []
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
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: false
        LockRotationY: false
        LockRotationZ: false
    MeshColliderComponent:
      AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\models\Sphere1m.fbx
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        - ID: 2380484423919574403
          ClassID: 17473292295039821981
          Enabled: true
          Fields:
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []