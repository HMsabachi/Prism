Scene: Scene Name
Environment:
  EnvironmentMap: 2825478391973848339
  Light:
    Direction: [-0.477, -1, -0.015]
    Radiance: [1, 1, 1]
    Multiplier: 0.515
  Shadow:
    Enabled: true
    Bias: 0
    NormalBias: 0.128
    CascadeCount: 4
    MaxDistance: 156.688
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
Entities:
  - Entity: 12290369221004411193
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Torus
    TransformComponent:
      Position: [3.8112779, 3.3402061, 5.2788844]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets/meshes/Default/Torus.fbx
    CSharpScriptComponent:
      Behaviours:
        - ClassID: 4883224417939687974
          Enabled: true
          Fields:
            - ID: 3811645116
              Name: mesh
              Type: 16
              Value: 0
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 5178862374589434728
    Parent: 3247025703490125974
    Children:
      []
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [0, 0.5, 0]
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
  - Entity: 7611555736437233215
    Parent: 0
    Children:
      []
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
      LinearDrag: 0
      AngularDrag: 0.05
      DisableGravity: false
      IsKinematic: false
      Layer: 3
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: false
        LockRotationY: false
        LockRotationZ: false
    BoxColliderComponent:
      Size: [1, 1, 1]
      Offset: [0, 0, 0]
      IsTrigger: false
      Material: 0
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 3247025703490125974
    Parent: 0
    Children:
      - Handle: 5178862374589434728
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [0.016000003, 1.5, 13.07]
      Rotation: [0, -0, 0]
      Scale: [2, 2, 2]
    MeshRendererComponent:
      AssetPath: Assets\meshes\Capsule.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
      LinearDrag: 0
      AngularDrag: 0.05
      DisableGravity: false
      IsKinematic: false
      Layer: 2
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: true
        LockRotationY: true
        LockRotationZ: true
    MeshColliderComponent:
      IsConvex: true
      IsTrigger: false
      OverrideMesh: false
      Material: 0
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
              Value: 21.7
            - ID: 868800460
              Name: JumpForce
              Type: 1
              Value: 5
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
        []
  - Entity: 8444794147831695495
    Parent: 0
    Children:
      []
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
      Intensity: 0.8
      CastShadows: true
      SoftShadows: true
      LightSize: 0.5
  - Entity: 3588066077231442219
    Parent: 0
    Children:
      []
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
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
      Material: 0
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 16798323008179040571
    Parent: 0
    Children:
      []
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
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
      Material: 0
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 16719024364629068537
    Parent: 0
    Children:
      []
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
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
      Material: 0
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
  - Entity: 8774112719293746857
    Parent: 0
    Children:
      []
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
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
      Material: 0
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 12245502073753085886
    Parent: 0
    Children:
      []
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
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
      Material: 0
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
  - Entity: 180212446636677419
    Parent: 0
    Children:
      []
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
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
      Material: 0
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 6685438149830436746
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-4.422, 14.876, -1]
      Rotation: [0, -0, 0]
      Scale: [2, 2, 2]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.5
      LinearDrag: 0
      AngularDrag: 0.05
      DisableGravity: false
      IsKinematic: false
      Layer: 3
      Constraints:
        LockPositionX: false
        LockPositionY: false
        LockPositionZ: false
        LockRotationX: false
        LockRotationY: false
        LockRotationZ: false
    BoxColliderComponent:
      Size: [1, 1, 1]
      Offset: [0, 0, 0]
      IsTrigger: false
      Material: 0
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
  - Entity: 6002286925489110848
    Parent: 0
    Children:
      []
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
    SphereColliderComponent:
      Radius: 0.5
      IsTrigger: false
      Material: 0
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 15961048996214737009
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Mesh
    TransformComponent:
      Position: [7.676742, 6.5393863, -5.0555487]
      Rotation: [0, -0, 0]
      Scale: [0.056000005, 0.056000005, 0.056000005]
    MeshRendererComponent:
      AssetPath: Assets\meshes\cerberus\CerberusMaterials.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 5
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
    MeshColliderComponent:
      IsConvex: false
      IsTrigger: false
      OverrideMesh: false
      Material: 0
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 9421913743984938135
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Sky Light
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [-21.190977, -0, 0]
      Scale: [1, 1, 1]
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
    SkyLightComponent:
      Intensity: 0.2
      Angle: 0
      SkyboxLod: 1
      EnvironmentMap: 2825478391973848339
  - Entity: 9087946811847071748
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Cube1m
    TransformComponent:
      Position: [-0.012693193, 4.9368415, 0.010737926]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets/meshes/Cube1m.fbx
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
    BoxColliderComponent:
      Size: [1, 1, 1]
      Offset: [0, 0, 0]
      IsTrigger: false
      Material: 0
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        - ClassID: 8019244560703880993
          Enabled: true
        - ClassID: 13335709507787291545
          Enabled: true
          Fields:
            - ID: 3811645116
              Name: mesh
              Type: 16
              Value: 0