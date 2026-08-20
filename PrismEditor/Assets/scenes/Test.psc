Scene: Scene Name
Environment:
  EnvironmentMap: 2825478391973848339
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
  - Entity: 9753572125788562516
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: sponza
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [0.02, 0.02, 0.02]
    MeshRendererComponent:
      AssetPath: Assets/meshes/sponza/sponza.obj
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []
  - Entity: 13776631932832522861
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
      EnvironmentMap: 2825478391973848339
  - Entity: 17335677611182898434
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Directional Light
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [-97.10017, -47.231075, 41.49542]
      Scale: [0.99999964, 0.9999993, 1]
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
  - Entity: 1295599128685309981
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Cube
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    MeshRendererComponent:
      AssetPath: Assets/meshes/Default/Cube.fbx
    CSharpScriptComponent:
      Behaviours:
        []
    PythonScriptComponent:
      Behaviours:
        []