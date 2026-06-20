Scene: Scene Name
Environment:
  AssetPath: assets\env\pink_sunrise_4k.hdr
  Light:
    Direction: [0.262, -1, -0.015]
    Radiance: [1, 1, 1]
    Multiplier: 1.143
  Shadow:
    Enabled: true
    Bias: 0.001
    NormalBias: 0.1
    CascadeCount: 4
PhysicsLayers:
  - Name: Default
    CollidesWith:
      []
Entities:
  - Entity: 11297686685644806108
    TagComponent:
      Tag: Mesh
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [17.2032, 17.2032, 17.2032]
    MeshRendererComponent:
      AssetPath: Assets\models\sponza\sponza.obj
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 480750975847703031
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [-23.693254, 1.5918453, -2.0246953e-06]
      Rotation: [0, 0, 0]
      Scale: [0.5, 1, 0.5]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      FixedRotation: true
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [0.25, 0.5]
      Density: 1.9
      Friction: 0.6
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 12498244675852797835
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-12.0348625, 6.596472, 9.600619e-07]
      Rotation: [0, 0, 0]
      Scale: [3.0000002, 0.3, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 0
      FixedRotation: false
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [1.5, 0.15]
      Density: 1
      Friction: 0.5
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
      Position: [-21.740631, 9.706595, 15]
      Rotation: [0.99991035, -0.013391121, 0, 0]
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
  - Entity: 1289165777996378215
    TagComponent:
      Tag: Cube
    TransformComponent:
      Position: [500, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1200, 1, 5]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 0
      FixedRotation: false
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [600, 0.5]
      Density: 1
      Friction: 0.5
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
      Position: [-16.412237, 3.1920667, -1.9073486e-06]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      FixedRotation: false
    CircleCollider2DComponent:
      Offset: [0, 0]
      Radius: 0.5
      Density: 1
      Friction: 1
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 1352995477042327524
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-29.680893, 29.75972, 0]
      Rotation: [0.70710677, 0, 0, 0.70710677]
      Scale: [58.417896, 4.479991, 4.48]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 0
      FixedRotation: false
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [29.7, 2.24]
      Density: 1
      Friction: 1
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 15223077898852293773
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [6.1267405, 45.561768, 0]
      Rotation: [0.97788364, 0, 0, -0.20914957]
      Scale: [4.4799967, 4.4799967, 4.48]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 1
      FixedRotation: false
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [2.24, 2.24]
      Density: 1
      Friction: 1
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 5421735812495444456
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-20.766222, 2.2943144, 0]
      Rotation: [0, 0, 0]
      Scale: [3.0000002, 0.3, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 0
      FixedRotation: false
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [1.5, 0.15]
      Density: 1
      Friction: 0.5
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 2842299641876190180
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [-16.614326, 4.39151, 6.433595e-09]
      Rotation: [0, 0, 0]
      Scale: [3.0000002, 0.3, 1]
    MeshRendererComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBody2DComponent:
      BodyType: 0
      FixedRotation: false
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [1.5, 0.15]
      Density: 1
      Friction: 0.5
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []