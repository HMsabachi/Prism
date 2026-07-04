Scene: Scene Name
Environment:
  EnvironmentMap: 9998254420492741016
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
  - Entity: 4437702480284700836
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: TransformTest
    TransformComponent:
      Position: [0, 2.4613252, 5.79594]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: Assets/meshes/Tests/TransformTest.fbx
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 6385741144206808496
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: TestScene
    TransformComponent:
      Position: [-3.6650949, 1.8218101, 0]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: Assets/meshes/TestScene.fbx
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 10252406678423167503
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Cylinder
    TransformComponent:
      Position: [6.996212, 3.6148028, 4.905213]
      Rotation: [0, -0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: Assets/meshes/Default/Cylinder.fbx
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 12701462489340545265
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Directional Light
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [21.408594, 30.226818, 57.53708]
      Scale: [0.9999999, 0.9998923, 0.9999461]
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
    DirectionalLightComponent:
      Radiance: [1, 1, 1]
      Intensity: 2
      CastShadows: true
      SoftShadows: true
      LightSize: 0.5
  - Entity: 6339274599783244545
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Torus
    TransformComponent:
      Position: [0, 8.116162, 0]
      Rotation: [0, -0, 0]
      Scale: [4.08, 4.08, 4.08]
    MeshRendererComponent:
      AssetPath: Assets/meshes/Default/Torus.fbx
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 480167885545322113
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Cube
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, -0, 0]
      Scale: [12, 1, 12.81]
    MeshRendererComponent:
      AssetPath: Assets/meshes/Default/Cube.fbx
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 11770117328004185642
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Sky Light
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0]
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
      SkyboxLod: 0
      EnvironmentMap: 9998254420492741016