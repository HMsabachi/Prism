Scene: Scene Name
Environment:
  AssetPath: assets\env\birchwood_4k.hdr
  Light:
    Direction: [-0.5, -0.5, 1]
    Radiance: [1, 1, 1]
    Multiplier: 1
Entities:
  - Entity: 9095450049242347594
    TagComponent:
      Tag: Test Entity
    TransformComponent:
      Position: [0.24810958, -1.9073486e-06, -0.268641]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    ScriptComponent:
      ModuleName: Example.Script
      StoredFields:
        - Name: SinkRate
          Type: 1
          Data: 0
        - Name: Speed
          Type: 1
          Data: 1
        - Name: VerticalSpeed
          Type: 1
          Data: 0
        - Name: Rotation
          Type: 1
          Data: 0
        - Name: Velocity
          Type: 6
          Data: [0, 0, 0]
    MeshComponent:
      AssetPath: assets\meshes\TestScene.fbx
  - Entity: 5178862374589434728
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [0, 14.75, 79.75]
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
  - Entity: 1289165777996378215
    TagComponent:
      Tag: Sphere
    TransformComponent:
      Position: [0, 21.980507, -1.6400628]
      Rotation: [1, 0, 0, 0]
      Scale: [0.100000024, 0.100000024, 0.100000024]
    ScriptComponent:
      ModuleName: Example.Sink
      StoredFields:
        - Name: SinkSpeed
          Type: 1
          Data: 5
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx