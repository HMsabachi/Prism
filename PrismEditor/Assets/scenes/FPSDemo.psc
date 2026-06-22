Scene: Scene Name
Environment:
  AssetPath: Assets\env\pink_sunrise_4k.hdr
  Light:
    Direction: [-0.477, -1, -0.015]
    Radiance: [1, 1, 1]
    Multiplier: 0.515
  Shadow:
    Enabled: true
    Bias: 0
    NormalBias: 0.128
    CascadeCount: 4
    MaxDistance: 242.851
PhysicsLayers:
  - Name: Default
    CollidesWith:
      []
  - Name: Player
    CollidesWith:
      - Name: Default
      - Name: Ground
  - Name: Ground
    CollidesWith:
      - Name: Default
      - Name: Player
      - Name: Cubes
  - Name: Cubes
    CollidesWith:
      - Name: Default
      - Name: Ground
  - Name: Default
    CollidesWith:
      []
  - Name: Player
    CollidesWith:
      []
  - Name: Ground
    CollidesWith:
      []
  - Name: Cubes
    CollidesWith:
      []
  - Name: Default
    CollidesWith:
      []
  - Name: Player
    CollidesWith:
      []
  - Name: Ground
    CollidesWith:
      []
  - Name: Cubes
    CollidesWith:
      []
  - Name: Default
    CollidesWith:
      []
  - Name: Player
    CollidesWith:
      []
  - Name: Ground
    CollidesWith:
      []
  - Name: Cubes
    CollidesWith:
      []
  - Name: Default
    CollidesWith:
      []
  - Name: Player
    CollidesWith:
      []
  - Name: Ground
    CollidesWith:
      []
  - Name: Cubes
    CollidesWith:
      []
Entities:
  - Entity: 5178862374589434728
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [2.808, 2.25, 0]
      Rotation: [0, 0, 0]
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
  - Entity: 7611555736437233215
    TagComponent:
      Tag: Ground
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, -0, 0]
      Scale: [30, 1, 30]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 0
      Mass: 1
      IsKinematic: false
      Layer: 3
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
  - Entity: 3247025703490125974
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [0.016, 1.5, 13.07]
      Rotation: [0, 0, 0]
      Scale: [2, 2, 2]
    MeshRendererComponent:
      AssetPath: Assets\meshes\Capsule.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      IsKinematic: false
      Layer: 2
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
      Bounciness: 0
    MeshColliderComponent:
      AssetPath: Assets\meshes\Capsule.fbx
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        - ClassID: 4621281492369714213
          Enabled: true
          Fields:
            - ID: 316217235
              Name: WalkingSpeed
              Type: 1
              Value: 15
            - ID: 2138685965
              Name: RunSpeed
              Type: 1
              Value: 20
            - ID: 868800460
              Name: JumpForce
              Type: 1
              Value: 100
            - ID: 1438023815
              Name: MouseSensitivity
              Type: 1
              Value: 8
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 8444794147831695495
    TagComponent:
      Tag: Directional Light
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [-8.657446, 17.98342, 26.237982]
      Scale: [1, 1, 1]
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
    DirectionalLightComponent:
      Radiance: [1, 1, 1]
      Intensity: 1.4
      CastShadows: true
      SoftShadows: true
      LightSize: 0.5
  - Entity: 3588066077231442219
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [-0.868, 21.017824, 1.6099999]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.7
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
      DynamicFriction: 0.25
      Bounciness: 0.5
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
  - Entity: 16798323008179040571
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [2.067, 16.320494, 6.495]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 5
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
      DynamicFriction: 0.25
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
  - Entity: 16719024364629068537
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [2.452, 24.187178, -3.581]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 5
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
      DynamicFriction: 0.25
      Bounciness: 0.5
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 8774112719293746857
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [2.322, 17.726751, -1.051]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.2
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
      DynamicFriction: 0.25
      Bounciness: 0.5
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 12245502073753085886
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [2.727, 19.70917, 2.272]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.2
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
      DynamicFriction: 0.25
      Bounciness: 0.4
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 180212446636677419
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [0.105, 10.336651, 3.277]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.9
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
      DynamicFriction: 0.25
      Bounciness: 0.5
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
  - Entity: 6685438149830436746
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-4.422, 14.876, -1]
      Rotation: [0, 0, 0]
      Scale: [2, 2, 2]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.5
      IsKinematic: false
      Layer: 3
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
      Bounciness: 1
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
  - Entity: 6002286925489110848
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [2.067, 21.957813, 0]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 2.9
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
      DynamicFriction: 0.25
      Bounciness: 0.5
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 15961048996214737009
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [7.676742, 11.546804, 6.605533]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 5
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
      DynamicFriction: 0.25
      Bounciness: 0.5
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 9421913743984938135
    TagComponent:
      Tag: Sky Light
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [-21.190977, 0, 0]
      Scale: [1, 1, 1]
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
    SkyLightComponent:
      Intensity: 1
      Angle: 0
      AssetPath: Assets\env\pink_sunrise_4k.hdr