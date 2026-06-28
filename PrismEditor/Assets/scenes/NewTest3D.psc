Scene: Scene Name
Environment:
  AssetPath: ""
  Light:
    Direction: [-0.5, -1, -0.5]
    Radiance: [1, 1, 1]
    Multiplier: 1
  Shadow:
    Enabled: true
    Bias: 0.001
    NormalBias: 0.1
    CascadeCount: 4
    MaxDistance: 200
PhysicsLayers:
  []
Entities:
  - Entity: 4155768779932166321
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [12.441303, 1.8964186, 0]
      Rotation: [0, -0, 0]
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
  - Entity: 8599118811544175475
    TagComponent:
      Tag: Cylinder
    TransformComponent:
      Position: [2.1474607, 1.3011947, -7.035705]
      Rotation: [0, -0, 0]
      Scale: [0.012400007, 0.012400007, 0.012400007]
    MeshRendererComponent:
      AssetPath: assets/meshes/Default/Cylinder.fbx
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 15345554648795530648
    TagComponent:
      Tag: Cube
    TransformComponent:
      Position: [0, -1.8848879, 0]
      Rotation: [0, -0, 0]
      Scale: [15.16655, 1, 15.522981]
    MeshRendererComponent:
      AssetPath: assets/meshes/Default/Cube.fbx
    RigidBodyComponent:
      BodyType: 0
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.05
      DisableGravity: false
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
      Size: [2, 2, 2]
      Offset: [0, 0, 0]
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 3530056565078259526
    TagComponent:
      Tag: Directional Light
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [-8.2, 70.6, 62.8]
      Scale: [1, 1, 1]
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
    DirectionalLightComponent:
      Radiance: [1, 1, 1]
      Intensity: 1
      CastShadows: true
      SoftShadows: true
      LightSize: 0.5
  - Entity: 3785092971802201435
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [0, 0.6566484, 0]
      Rotation: [0, -0, 0]
      Scale: [0.01, 0.01, 0.01]
    MeshRendererComponent:
      AssetPath: assets/meshes/Default/Sphere.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.05
      DisableGravity: false
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
      Radius: 100
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
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
              Value: 5
            - ID: 1585848256
              Name: CameraForwardOffset
              Type: 1
              Value: 0.2
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
            - ID: 2756558778
              Name: CameraYOffset
              Type: 1
              Value: 0.85
            - ID: 1438023815
              Name: MouseSensitivity
              Type: 1
              Value: 10
    PythonScriptComponent:
      Behaviours:
        []