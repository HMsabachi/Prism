Scene: Scene Name
Environment:
  AssetPath: assets\env\pink_sunrise_4k.hdr
  Light:
    Direction: [-0.787, -0.733, 1]
    Radiance: [1, 1, 1]
    Multiplier: 0.515
Entities:
  - Entity: 15861629587505754
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-18.209566, 39.251823, 0]
      Rotation: [0.9670565, 0, 0, -0.2545618]
      Scale: [4.4799953, 4.4799953, 4.48]
    ScriptComponent:
      ModuleName: Example.RandomColor
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      Mass: 1
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [2.24, 2.24]
  - Entity: 15223077898852293773
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [5.3711987, 43.87629, 0]
      Rotation: [0.97788364, 0, 0, -0.20914969]
      Scale: [4.4799967, 4.4799967, 4.48]
    ScriptComponent:
      ModuleName: Example.RandomColor
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      Mass: 1
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [2.24, 2.24]
  - Entity: 2157107598622182863
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-7.6041155, 44.14422, 0]
      Rotation: [0.9892858, 0, 0, 0.14599171]
      Scale: [4.479993, 4.479993, 4.48]
    ScriptComponent:
      ModuleName: Example.RandomColor
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      Mass: 0.5
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [2.24, 2.24]
  - Entity: 8080964283681139153
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-0.7392117, 37.765327, 0]
      Rotation: [0.9564759, 0, 0, -0.29181132]
      Scale: [5, 2, 2]
    ScriptComponent:
      ModuleName: Example.RandomColor
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      Mass: 0.25
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [2.5, 1]
  - Entity: 1352995477042327524
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-8.329699, 30.407816, 0]
      Rotation: [0.78159535, 0, 0, 0.6237858]
      Scale: [14.000001, 4.4799933, 4.48]
    ScriptComponent:
      ModuleName: Example.RandomColor
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      Mass: 3
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [7, 2.24]
  - Entity: 935615878363259513
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [6.880312, 31.942337, 0]
      Rotation: [0.9865783, 0, 0, 0.16328894]
      Scale: [4.4799995, 4.4799995, 4.48]
    ScriptComponent:
      ModuleName: Example.RandomColor
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      Mass: 1
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [2.24, 2.24]
  - Entity: 14057422478420564497
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [0, 22.774044, 0]
      Rotation: [0.9425914, 0, 0, -0.33394822]
      Scale: [6.0000005, 6.0000005, 4.48]
    ScriptComponent:
      ModuleName: Example.PlayerCube
      StoredFields:
        - Name: HorizontalForce
          Type: 1
          Data: 10
        - Name: VerticalForce
          Type: 1
          Data: 10
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      Mass: 4.3
    CircleCollider2DComponent:
      Offset: [0, 0]
      Radius: 3
  - Entity: 1289165777996378215
    TagComponent:
      Tag: Cube
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [50, 1, 50]
    ScriptComponent:
      ModuleName: Example.RandomColor
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 0
      Mass: 1
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [25, 0.5]
  - Entity: 5178862374589434728
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [0, 25, 79.75]
      Rotation: [0.99560297, -0.09367391, 0, 0]
      Scale: [1, 0.9999998, 0.9999998]
    ScriptComponent:
      ModuleName: Example.BasicController
      StoredFields:
        - Name: Speed
          Type: 1
          Data: 12
    CameraComponent:
      Camera: some camera data...
      Primary: true
  - Entity: 3948844418381294888
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-1.4802856, 49.594524, -2.3841858e-07]
      Rotation: [0.97788364, 0, 0, -0.2091497]
      Scale: [1.9999996, 1.9999995, 2]
    ScriptComponent:
      ModuleName: Example.RandomColor
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      Mass: 1
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [1, 1]