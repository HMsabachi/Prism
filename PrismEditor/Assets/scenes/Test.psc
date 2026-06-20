Scene: Scene Name
Environment:
  AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\env\birchwood_4k.hdr
  Light:
    Direction: [-0.5, -1, -0.5]
    Radiance: [1, 1, 1]
    Multiplier: 1
Entities:
  - Entity: 4272325128816557942
    TagComponent:
      Tag: Noise
    TransformComponent:
      Position: [0, 6.5, 0]
      Rotation: [0, 0, 0]
      Scale: [9.25, 1, 10.25]
    ScriptComponent:
      ModuleName: Example.MapGenerator
      StoredFields:
        - Name: MapWidth
          Type: 2
          Data: 128
        - Name: MapHeight
          Type: 2
          Data: 128
        - Name: Offset
          Type: 5
          Data: [13.4, 6.26]
        - Name: Octaves
          Type: 2
          Data: 4
        - Name: Seed
          Type: 2
          Data: 21
        - Name: Persistance
          Type: 1
          Data: 0.74
        - Name: NoiseScale
          Type: 1
          Data: 0.5
        - Name: Lacunarity
          Type: 1
          Data: 3
  - Entity: 13982802161734492964
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [26.25, 15.75, 0]
      Rotation: [0.67942274, -0.19206704, 0.68146056, 0.19263057]
      Scale: [0.9999982, 0.9999893, 0.9999906]
    CameraComponent:
      Camera: some camera data...
      Primary: true
  - Entity: 10654856600807563614
    TagComponent:
      Tag: Empty Entity
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    ScriptComponent:
      ModuleName: Example.Script
      StoredFields:
        - Name: Speed
          Type: 1
          Data: 5
    MeshRendererComponent:
      AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\meshes\TestScene.fbx
  - Entity: 10266587003235082940
    TagComponent:
      Tag: ScriptNoise
    TransformComponent:
      Position: [0, 6.8021007, -8.643346]
      Rotation: [0, 0, 0]
      Scale: [10.0599985, 10.0599985, 10.0599985]
    ScriptComponent:
      ModuleName: Example.ScriptNoise
      StoredFields:
        []
    MeshRendererComponent:
      AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\models\Plane1m.obj