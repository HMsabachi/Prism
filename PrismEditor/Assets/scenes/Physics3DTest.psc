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
PhysicsLayers:
  - Name: Default
    CollidesWith:
      - Name: Default
      - Name: Default
Entities:
  - Entity: 3509336336274569647
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [0.57714844, 3.7860096, -9.399623]
      Rotation: [1, 0, 0, 0]
      Scale: [0.022940006, 0.022940006, 0.022940006]
    MeshComponent:
      AssetPath: Assets\models\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
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
    MeshColliderComponent:
      AssetPath: Assets\models\Sphere1m.fbx
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        - ClassID: 17473292295039821981
          Enabled: true
          Fields:
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
      Position: [-7.223452, 1.5, 0.4699059]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    MeshComponent:
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
        []
    PythonScriptComponent:
      Behaviours:
        - ClassID: 8019244560703880993
          Enabled: true
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
      Layer: 0
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
        - ClassID: 14697641082096727597
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
        - ClassID: 18172894909676220513
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
        - ClassID: 502499430819245105
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
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 4585746309842579712
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
        []
    PythonScriptComponent:
      Behaviours:
        - ClassID: 8019244560703880993
          Enabled: true
  - Entity: 11192140000393742823
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-3.8477411, 1.5, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    MeshComponent:
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
        []
    PythonScriptComponent:
      Behaviours:
        - ClassID: 8019244560703880993
          Enabled: true
  - Entity: 13919631429494874460
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [0.07178551, 1.5, -3.1342356]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    MeshComponent:
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
        []
    PythonScriptComponent:
      Behaviours:
        - ClassID: 8019244560703880993
          Enabled: true
  - Entity: 7159763310276242877
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-4.0302086, 1.5, -3.1342356]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    MeshComponent:
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
        []
    PythonScriptComponent:
      Behaviours:
        - ClassID: 8019244560703880993
          Enabled: true
  - Entity: 6217240602347854998
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-7.223452, 1.5, -3.1342356]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    MeshComponent:
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
        []
    PythonScriptComponent:
      Behaviours:
        - ClassID: 8019244560703880993
          Enabled: true